/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <algorithm>
#include <cmath>

#include <QPainterPath>

#include "qsx/config.hpp"
#include "qsx/curve_editor.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"

namespace qsx
{

CurveEditor::CurveEditor(const std::string &label_, int sample_count_, QWidget *parent)
    : QWidget(parent), label(label_), sample_count(sample_count_)
{
  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);

  this->clear_points();
}

void CurveEditor::clear_points()
{
  this->control_points.clear();
  this->control_points = {{0.f, 0.f}, {1.f, 1.f}};
  this->update_values();
}

void CurveEditor::draw_background(QPainter &painter)
{
  const int radius = QSX_CONFIG->global.radius;
  const int padding = QSX_CONFIG->global.padding;

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(), radius, radius);

  // bounding box
  QRectF area = this->rect().adjusted(padding, padding, -padding, -padding);
  painter.setPen(QSX_CONFIG->global.color_border);
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(area);
}

void CurveEditor::draw_curve(QPainter &painter)
{
  painter.setPen(QPen(QSX_CONFIG->global.color_border));
  painter.setBrush(Qt::NoBrush);

  QPainterPath path;
  QPointF      p0 = this->point_to_screen(this->control_points.front());
  path.moveTo(p0);

  // lines
  for (int k = 1; k < this->sample_count; ++k)
  {
    float   t = SFLOAT(k) / SFLOAT(this->sample_count - 1);
    float   y = this->interpolate(t);
    QPointF p = this->point_to_screen(QPointF(t, y));
    path.lineTo(p);
  }
  painter.drawPath(path);

  // sampling points
  if (QSX_CONFIG->curve.draw_sampling_points)
  {
    painter.setBrush(QBrush(QSX_CONFIG->global.color_border));

    for (int k = 0; k < this->sample_count; ++k)
    {
      float   t = SFLOAT(k) / SFLOAT(this->sample_count - 1);
      float   y = this->interpolate(t);
      QPointF p = this->point_to_screen(QPointF(t, y));
      painter.drawEllipse(p,
                          QSX_CONFIG->curve.sampling_point_radius,
                          QSX_CONFIG->curve.sampling_point_radius);
    }
  }
}

void CurveEditor::draw_points(QPainter &painter)
{
  painter.setPen(QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));
  painter.setBrush(QSX_CONFIG->global.color_bg);

  for (auto &p : this->control_points)
  {
    QPointF screen_p = this->point_to_screen(p);
    painter.drawEllipse(screen_p,
                        QSX_CONFIG->canvas.point_radius,
                        QSX_CONFIG->canvas.point_radius);
  }
}

bool CurveEditor::event(QEvent *event)
{
  switch (event->type())
  {
  case QEvent::HoverEnter:
  {
    this->is_hovered = true;
    this->update();
  }
  break;

  case QEvent::HoverLeave:
  {
    this->is_hovered = false;
    this->update();
  }
  break;

  default:
    break;
  }

  return QWidget::event(event);
}

int CurveEditor::find_nearest_point(const QPoint &pos) const
{
  const float radius = SFLOAT(QSX_CONFIG->global.radius);

  for (int i = 0; i < SINT(this->control_points.size()); ++i)
  {
    QPointF screen_p = this->point_to_screen(this->control_points[i]);
    if (QLineF(screen_p, pos).length() < 2 * radius)
      return i;
  }
  return -1;
}

int CurveEditor::get_sample_count() const { return SINT(this->values.size()); }

bool CurveEditor::get_smooth_interpolation() const { return this->smooth_interpolation; }

std::vector<float> CurveEditor::get_values() const { return this->values; }

float CurveEditor::interpolate(float t) const
{
  if (this->smooth_interpolation)
    return this->interpolate_catmull_rom(t);
  else
    return this->interpolate_linear(t);
}

float CurveEditor::interpolate_catmull_rom(float t) const
{
  if (this->control_points.empty())
    return 0.f;
  if (t <= this->control_points.front().x())
    return SFLOAT(this->control_points.front().y());
  if (t >= this->control_points.back().x())
    return SFLOAT(this->control_points.back().y());

  // Find segment
  int segment = -1;
  for (int i = 0; i < int(this->control_points.size()) - 1; ++i)
  {
    if (t >= this->control_points[i].x() && t <= this->control_points[i + 1].x())
    {
      segment = i;
      break;
    }
  }
  if (segment < 0)
    return SFLOAT(this->control_points.back().y());

  // Get control points p0, p1, p2, p3 for Catmull-Rom
  auto p0 = this->control_points[std::max(0, segment - 1)];
  auto p1 = this->control_points[segment];
  auto p2 = this->control_points[segment + 1];
  auto p3 = this->control_points[std::min(segment + 2,
                                          int(this->control_points.size()) - 1)];

  // Normalize local parameter u in [0, 1]
  float u = SFLOAT((t - p1.x()) / (p2.x() - p1.x()));
  u = std::clamp(u, 0.0f, 1.0f);

  // Catmull–Rom spline formula
  float u2 = u * u;
  float u3 = u2 * u;

  float y = SFLOAT(0.5f * ((2.0f * p1.y()) + (-p0.y() + p2.y()) * u +
                           (2.0f * p0.y() - 5.0f * p1.y() + 4.0f * p2.y() - p3.y()) * u2 +
                           (-p0.y() + 3.0f * p1.y() - 3.0f * p2.y() + p3.y()) * u3));

  return std::clamp(y, 0.0f, 1.0f); // clamp for safety
}

float CurveEditor::interpolate_linear(float t) const
{
  // linear interpolation between nearest control points
  if (this->control_points.empty())
    return 0.0f;
  if (t <= this->control_points.front().x())
    return SFLOAT(this->control_points.front().y());
  if (t >= this->control_points.back().x())
    return SFLOAT(this->control_points.back().y());

  for (size_t i = 0; i < this->control_points.size() - 1; ++i)
  {
    auto &p0 = this->control_points[i];
    auto &p1 = this->control_points[i + 1];
    if (t >= p0.x() && t <= p1.x())
    {
      float u = SFLOAT((t - p0.x()) / (p1.x() - p0.x()));
      return SFLOAT(p0.y() * (1 - u) + p1.y() * u);
    }
  }
  return 0.0f;
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    int idx = this->find_nearest_point(event->pos());
    if (idx >= 0)
    {
      this->active_point = idx;
      this->is_dragging = true;
    }
    else
    {
      QPointF new_point = this->screen_to_point(event->pos());
      this->control_points.push_back(new_point);
      std::sort(this->control_points.begin(),
                this->control_points.end(),
                [](auto &a, auto &b) { return a.x() < b.x(); });
      this->update_values();
    }
  }
  else if (event->button() == Qt::RightButton)
  {
    int idx = this->find_nearest_point(event->pos());
    // do not allow removal of first and last control points
    if (idx > 0 && idx < SINT(this->control_points.size()) - 1)
      this->control_points.erase(this->control_points.begin() + idx);
    this->update_values();
  }
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging && this->active_point >= 0)
  {
    QPointF new_pos = this->screen_to_point(event->pos());

    // prevent "x" modification for first and last control points
    if (this->active_point == 0 ||
        this->active_point == SINT(this->control_points.size()) - 1)
      new_pos.setX(this->control_points[this->active_point].x());

    this->control_points[this->active_point] = new_pos;

    std::sort(this->control_points.begin(),
              this->control_points.end(),
              [](auto &a, auto &b) { return a.x() < b.x(); });

    // update active point index after sorting
    auto it = std::find_if(this->control_points.begin(),
                           this->control_points.end(),
                           [new_pos](auto &p)
                           { return p.x() == new_pos.x() && p.y() == new_pos.y(); });

    this->active_point = SINT(std::distance(this->control_points.begin(), it));

    this->update_values();
  }
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  this->is_dragging = false;
  this->active_point = -1;
  Q_EMIT this->edit_ended();
}

void CurveEditor::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  this->draw_background(painter);

  // label
  if (!this->label.empty())
  {
    QFontMetrics fm(this->font());
    int          base_dx = text_width(this, "M");
    int          base_dy = fm.height() + 2 * QSX_CONFIG->global.padding;

    QRect rect_label = QRect(QPoint(base_dx, 0),
                             QSize(this->rect().width() - base_dx, base_dy));

    painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
    painter.setPen(QPen(QSX_CONFIG->global.color_text));
    painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());
  }

  this->draw_curve(painter);
  this->draw_points(painter);
}

QPointF CurveEditor::point_to_screen(const QPointF &p) const
{
  const int padding = QSX_CONFIG->global.padding;

  float pf = SFLOAT(padding);
  float wf = SFLOAT(this->width() - 2 * padding);
  float hf = SFLOAT(this->height() - 2 * padding);

  return QPointF(pf + SFLOAT(p.x()) * wf, pf + (1.f - SFLOAT(p.y())) * hf);
}

QPointF CurveEditor::screen_to_point(const QPoint &p) const
{
  const int padding = QSX_CONFIG->global.padding;
  float     wf = SFLOAT(this->width() - 2 * padding);
  float     hf = SFLOAT(this->height() - 2 * padding);

  return QPointF(std::clamp(SFLOAT(p.x() - padding) / wf, 0.f, 1.f),
                 std::clamp(1.f - SFLOAT(p.y() - padding) / hf, 0.f, 1.f));
}

void CurveEditor::set_sample_count(int new_sample_count)
{
  this->sample_count = new_sample_count;
  this->update_values();
  this->update();

  Q_EMIT this->edit_ended();
}

void CurveEditor::set_smooth_interpolation(bool new_state)
{
  this->smooth_interpolation = new_state;
  this->update_values();
  Q_EMIT this->edit_ended();
}

void CurveEditor::set_values(const std::vector<float> &new_values)
{
  this->values = new_values;
  this->sample_count = SINT(this->values.size());
  this->update();
}

QSize CurveEditor::sizeHint() const
{
  return {QSX_CONFIG->global.width_min,
          SINT(0.5f * SFLOAT(QSX_CONFIG->global.width_min))};
}

void CurveEditor::update_values()
{
  this->values.resize(this->sample_count);
  for (int i = 0; i < this->sample_count; ++i)
  {
    float t = float(i) / SFLOAT(this->sample_count - 1);
    this->values[i] = this->interpolate(t);
  }
  this->update();
  Q_EMIT this->value_changed();
}

} // namespace qsx
