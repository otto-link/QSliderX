/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <format>

#include <QEvent>
#include <QHoverEvent>
#include <QPainter>
#include <QtMath>

#include "qsx/canvas_points.hpp"
#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"

namespace qsx
{

CanvasPoints::CanvasPoints(const std::string &label_,
                           float              xmin_,
                           float              xmax_,
                           float              ymin_,
                           float              ymax_,
                           const std::string &value_format_,
                           QWidget           *parent)
    : QWidget(parent), xmin(xmin_), xmax(xmax_), ymin(ymin_), ymax(ymax_),
      value_format(value_format_)
{
  QSXLOG->trace("CanvasPoints::CanvasPoints");

  this->label = truncate_string(label_,
                                static_cast<size_t>(QSX_CONFIG->global.max_label_len));

  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->update_geometry();
}

void CanvasPoints::add_point(float x, float y)
{
  this->points_x.push_back(x);
  this->points_y.push_back(y);
  this->points_z.push_back(1.f);
  this->update();
  Q_EMIT this->value_changed();
}

void CanvasPoints::canvas_position_to_xy(QPoint pos, float &x, float &y)
{
  pos -= this->rect_points.topLeft();

  float rx = SFLOAT(pos.x()) / SFLOAT(this->rect_points.width());
  float ry = 1.f - SFLOAT(pos.y()) / SFLOAT(this->rect_points.height());

  x = this->xmin + rx * (this->xmax - this->xmin);
  y = this->ymin + ry * (this->ymax - this->ymin);
}

bool CanvasPoints::event(QEvent *event)
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
    this->hovered_point_id = -1;
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    auto  *hover = static_cast<QHoverEvent *>(event);
    QPoint mouse_pos = hover->position().toPoint(); // mouse position inside the widget

    if (!this->is_dragging)
    {
      this->hovered_point_id = -1;
      int radius = QSX_CONFIG->canvas.point_radius;
      for (size_t k = 0; k < this->points_x.size(); ++k)
      {
        QPoint pos = this->xy_to_canvas_position(this->points_x[k], this->points_y[k]);
        QRect  prect = QRect(pos - QPoint(radius, radius), QSize(2 * radius, 2 * radius));
        if (prect.contains(mouse_pos))
          this->hovered_point_id = SINT(k);
      }
    }

    this->update();
  }
  break;

  default:
    break;
  }

  return QWidget::event(event);
}

std::vector<float> CanvasPoints::get_points_x() const { return this->points_x; }

std::vector<float> CanvasPoints::get_points_y() const { return this->points_y; }

std::vector<float> CanvasPoints::get_points_z() const { return this->points_z; }

std::string CanvasPoints::get_value_as_string(float v) const
{
  return std::vformat(this->value_format, std::make_format_args(v));
}

void CanvasPoints::mouseDoubleClickEvent(QMouseEvent *event)
{
  QPoint pos = event->position().toPoint();

  if (this->rect_points.contains(pos))
  {
    float x, y;
    this->canvas_position_to_xy(pos, x, y);
    this->add_point(x, y);
    // focus on new pt
    this->hovered_point_id = SINT(this->points_x.size() - 1);
    Q_EMIT this->edit_ended();
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void CanvasPoints::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    // pixels per unit
    float ppu_x = SFLOAT(this->rect_points.width()) / (this->xmax - this->xmin);
    float ppu_y = SFLOAT(this->rect_points.height()) / (this->ymax - this->ymin);

    if (event->modifiers() & Qt::ControlModifier)
    {
      ppu_x *= QSX_CONFIG->canvas.ppu_multiplier_fine_tuning;
      ppu_y *= QSX_CONFIG->canvas.ppu_multiplier_fine_tuning;
    }

    QPoint delta = event->position().toPoint() - this->mouse_pos_before_dragging;
    float  dvx = SFLOAT(delta.x()) / ppu_x;
    float  dvy = SFLOAT(delta.y()) / ppu_y;

    this->points_x[this->hovered_point_id] = this->value_x_before_dragging + dvx;
    this->points_y[this->hovered_point_id] = this->value_y_before_dragging - dvy;

    // check bounds
    this->points_x[this->hovered_point_id] = std::clamp(
        this->points_x[this->hovered_point_id],
        this->xmin,
        this->xmax);
    this->points_y[this->hovered_point_id] = std::clamp(
        this->points_y[this->hovered_point_id],
        this->ymin,
        this->ymax);

    this->update();

    event->accept();

    Q_EMIT this->value_changed();
  }

  QWidget::mouseMoveEvent(event);
}

void CanvasPoints::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->hovered_point_id >= 0)
    {
      this->value_x_before_dragging = this->points_x[this->hovered_point_id];
      this->value_y_before_dragging = this->points_y[this->hovered_point_id];
      this->mouse_pos_before_dragging = event->position().toPoint();
      this->set_is_dragging(true);
    }
  }
  else if (event->button() == Qt::RightButton)
  {
    if (this->hovered_point_id >= 0)
    {
      int id_to_remove = this->hovered_point_id;
      this->hovered_point_id = -1; // before update
      this->remove_point(id_to_remove);
      Q_EMIT this->edit_ended();
    }
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void CanvasPoints::mouseReleaseEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    this->set_is_dragging(false);
    Q_EMIT this->edit_ended();
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void CanvasPoints::paintEvent(QPaintEvent *)
{
  const int radius = QSX_CONFIG->global.radius;
  const int point_radius = QSX_CONFIG->canvas.point_radius;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(), radius, radius);

  // background image
  if (!this->bg_image.isNull())
    painter.drawImage(this->rect_points, this->bg_image);

  // labels
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  painter.drawText(this->rect_label,
                   Qt::AlignLeft | Qt::AlignVCenter,
                   this->label.c_str());

  // points
  for (size_t k = 0; k < this->points_x.size(); ++k)
  {
    bool toggle = (SINT(k) == this->hovered_point_id);

    QPoint pos = this->xy_to_canvas_position(this->points_x[k], this->points_y[k]);

    // point value
    if (this->draw_z_value)
    {
      int arc_width = QSX_CONFIG->canvas.value_arc_width;

      painter.setPen(Qt::NoPen);
      painter.setBrush(QBrush(QSX_CONFIG->global.color_faded));

      QPoint delta = QPoint(-point_radius - arc_width, -point_radius - arc_width);
      int    lx = 2 * (point_radius + arc_width);

      int   alpha = SINT(this->points_z[k] * 360.f * 16.f);
      QRect rect_arc = QRect(pos + delta, QSize(lx, lx));
      painter.drawPie(rect_arc, 90 * 16 - alpha, alpha);
    }

    // point itself
    painter.setPen(QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));
    painter.setBrush(toggle ? QBrush(QSX_CONFIG->global.color_selected)
                            : QBrush(QSX_CONFIG->global.color_bg));

    painter.drawEllipse(pos, point_radius, point_radius);

    // connections
    if (this->connected_points && k < this->points_x.size() - 1)
    {
      painter.setPen(
          QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));
      painter.setBrush(Qt::NoBrush);

      QPoint pos_next = this->xy_to_canvas_position(this->points_x[k + 1],
                                                    this->points_y[k + 1]);
      painter.drawLine(pos, pos_next);
    }
  }

  // display value if dragging
  if (this->hovered_point_id >= 0) // this->is_dragging)
  {
    painter.setPen(QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));

    std::string sx = this->get_value_as_string(this->points_x[this->hovered_point_id]);
    std::string sy = this->get_value_as_string(this->points_y[this->hovered_point_id]);
    std::string sz = this->get_value_as_string(this->points_z[this->hovered_point_id]);
    std::string txt;

    if (this->draw_z_value)
      txt = "(" + sx + ", " + sy + ", " + sz + ")";
    else
      txt = "(" + sx + ", " + sy + ")";

    QPoint pos = this->xy_to_canvas_position(this->points_x[this->hovered_point_id],
                                             this->points_y[this->hovered_point_id]);

    resize_font(this, -2);
    int w = text_width(this, txt);
    int h = text_height(this);
    int dy = h + QSX_CONFIG->canvas.point_radius + QSX_CONFIG->canvas.value_arc_width;

    // keep text within the visible rectangle
    QPoint text_pos = pos + QPoint(0, -dy);
    text_pos.setX(std::min(text_pos.x(), this->rect_points.right() - w));
    if (text_pos.y() <= 0)
      text_pos.setY(pos.y() + dy - h);

    painter.setFont(this->font());
    painter.drawText(QRect(text_pos, QSize(w, h)),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     txt.c_str());

    resize_font(this, 2);
    painter.setFont(this->font());
  }
}

void CanvasPoints::remove_point(int idx)
{
  this->points_x.erase(this->points_x.begin() + idx);
  this->points_y.erase(this->points_y.begin() + idx);
  this->points_z.erase(this->points_z.begin() + idx);
  this->update();
  Q_EMIT this->value_changed();
}

void CanvasPoints::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void CanvasPoints::set_bg_image(const QImage &new_bg_image)
{
  this->bg_image = new_bg_image.copy();
  this->update_geometry();
  this->update();
}

void CanvasPoints::set_connected_points(bool new_state)
{
  this->connected_points = new_state;
  this->update();
}

void CanvasPoints::set_draw_z_value(bool new_state)
{
  this->draw_z_value = new_state;
  this->update();
}

void CanvasPoints::set_points(const std::vector<float> &new_x,
                              const std::vector<float> &new_y)
{
  this->set_points_x(new_x);
  this->set_points_y(new_y);

  std::vector<float> new_z(new_x.size(), 1.f);
  this->set_points_z(new_z);
}

void CanvasPoints::set_points_x(const std::vector<float> &new_x)
{
  this->points_x = new_x;
  this->update();
  Q_EMIT this->value_changed();
}

void CanvasPoints::set_points_y(const std::vector<float> &new_y)
{
  this->points_y = new_y;
  this->update();
  Q_EMIT this->value_changed();
}

void CanvasPoints::set_points_z(const std::vector<float> &new_z)
{
  this->points_z = new_z;
  this->update();
  Q_EMIT this->value_changed();
}

void CanvasPoints::set_is_dragging(bool new_state)
{
  this->is_dragging = new_state;

  if (this->is_dragging)
    this->setCursor(Qt::CrossCursor);
  else
    this->setCursor(Qt::ArrowCursor);
}

QSize CanvasPoints::sizeHint() const
{
  return QSize(this->canvas_width, this->canvas_height);
}

void CanvasPoints::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = text_width(this, "M");
  this->base_dy = fm.height() + QSX_CONFIG->slider.padding_v;

  int label_width = 2 * this->base_dx + text_width(this, this->label);

  this->canvas_width = 256;
  this->canvas_height = 256;

  // keep background image aspect ratio (if any)
  if (!this->bg_image.isNull())
  {
    float aspect_ratio = SFLOAT(this->bg_image.width()) / SFLOAT(this->bg_image.height());

    if (aspect_ratio < 1.f)
    {
      this->resize(SINT(SFLOAT(this->rect().height()) * aspect_ratio),
                   this->rect().height());
      this->canvas_width = SINT(SFLOAT(this->canvas_height) * aspect_ratio);
    }
    else
    {
      this->resize(this->rect().width(),
                   SINT(SFLOAT(this->rect().width()) / aspect_ratio));
      this->canvas_height = SINT(SFLOAT(this->canvas_width) / aspect_ratio);
    }
  }

  // size
  this->setMinimumWidth(this->canvas_width);
  this->setMinimumHeight(this->canvas_height);

  this->rect_points = this->rect().adjusted(this->base_dx,
                                            this->base_dy,
                                            -this->base_dx,
                                            -this->base_dx);
  this->rect_points = this->rect();
  this->rect_label = QRect(QPoint(this->base_dx, 0),
                           QSize(this->rect().width() - this->base_dx, this->base_dy));
}

QPoint CanvasPoints::xy_to_canvas_position(float x, float y)
{
  float range_x = this->xmax - this->xmin;
  float range_y = this->ymax - this->ymin;
  float rx = (x - this->xmin) / range_x;
  float ry = (y - this->ymin) / range_y;

  int px = this->rect_points.left() + SINT(rx * SFLOAT(this->rect_points.width()));
  int py = this->rect_points.top() +
           SINT((1.f - ry) * SFLOAT(this->rect_points.height()));

  return QPoint(px, py);
}

void CanvasPoints::wheelEvent(QWheelEvent *event)
{
  if (this->hovered_point_id >= 0 && this->draw_z_value)
  {
    float diff = QSX_CONFIG->canvas.wheel_diff;
    if (event->modifiers() & Qt::ControlModifier)
      diff /= QSX_CONFIG->canvas.wheel_multiplier_fine_tuning;

    float delta = event->angleDelta().y() > 0 ? diff : -diff;
    this->points_z[this->hovered_point_id] += delta;
    this->points_z[this->hovered_point_id] = std::clamp(
        this->points_z[this->hovered_point_id],
        0.f,
        1.f);

    this->update();

    Q_EMIT this->value_changed();
    Q_EMIT this->edit_ended();
  }
}

} // namespace qsx
