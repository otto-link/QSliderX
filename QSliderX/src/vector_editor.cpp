/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <algorithm>

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"
#include "qsx/vector_editor.hpp"

namespace qsx
{

VectorEditor::VectorEditor(const std::string &label_,
                           std::vector<float> values_,
                           QWidget           *parent)
    : QWidget(parent), label(label_), values(values_)
{
  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
}

bool VectorEditor::event(QEvent *event)
{
  if (event->type() == QEvent::HoverEnter)
  {
    this->is_hovered = true;
    this->update();
  }
  else if (event->type() == QEvent::HoverLeave)
  {
    this->is_hovered = false;
    this->update();
  }
  return QWidget::event(event);
}

const std::vector<float> &VectorEditor::get_values() const { return values; }

QPointF VectorEditor::index_to_screen(int i) const
{
  QRectF a = this->value_area();
  float  x = float(a.left() + float(i) / float(this->values.size() - 1) * a.width());
  float  y = float(a.top() + (1.f - this->values[i]) * a.height());
  return {x, y};
}

void VectorEditor::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    int idx = this->screen_to_index(event->pos().x());
    if (idx >= 0)
    {
      this->active_index = idx;
      this->is_dragging = true;

      float new_val = this->screen_to_value(event->pos().y());
      this->values[idx] = new_val;

      this->update();
      Q_EMIT this->value_changed();
    }
  }
}

void VectorEditor::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging && this->active_index >= 0)
  {
    float new_val = this->screen_to_value(event->pos().y());
    this->values[this->active_index] = new_val;
    this->update();
    Q_EMIT this->value_changed();
  }
}

void VectorEditor::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  this->is_dragging = false;
  this->active_index = -1;
  Q_EMIT this->edit_ended();
}

void VectorEditor::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const int radius = QSX_CONFIG->global.radius;
  const int padding = QSX_CONFIG->global.padding;

  // background
  p.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  p.setPen(this->is_hovered
               ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
               : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  p.drawRoundedRect(this->rect(), radius, radius);

  // bounding box
  QRectF area = this->rect().adjusted(padding, padding, -padding, -padding);
  p.setPen(QSX_CONFIG->global.color_border);
  p.setBrush(Qt::NoBrush);
  p.drawRect(area);

  // label
  if (!this->label.empty())
  {
    QFontMetrics fm(this->font());
    int          base_dx = text_width(this, "M");
    int          base_dy = fm.height() + 2 * QSX_CONFIG->global.padding;

    QRect rect_label = QRect(QPoint(base_dx, 0),
                             QSize(this->rect().width() - base_dx, base_dy));

    p.setBrush(QBrush(QSX_CONFIG->global.color_text));
    p.setPen(QPen(QSX_CONFIG->global.color_text));
    p.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());
  }

  // line between points
  p.setPen(QPen(QSX_CONFIG->global.color_border));
  p.setBrush(Qt::NoBrush);

  QPainterPath path;
  path.moveTo(this->index_to_screen(0));

  for (int i = 1; i < int(this->values.size()); ++i)
    path.lineTo(this->index_to_screen(i));

  p.drawPath(path);

  // points
  p.setPen(QPen(QSX_CONFIG->global.color_text));
  p.setBrush(QBrush(QSX_CONFIG->global.color_border));
  for (int i = 0; i < int(this->values.size()); ++i)
    p.drawEllipse(this->index_to_screen(i), radius, radius);
}

float VectorEditor::screen_to_value(int y) const
{
  QRectF a = this->value_area();
  float  v = float(1.f - float(y - a.top()) / a.height());
  return std::clamp(v, 0.f, 1.f);
}

int VectorEditor::screen_to_index(int x) const
{
  QRectF a = this->value_area();
  if (this->values.empty())
    return -1;

  float t = float(x - a.left()) / float(a.width());
  int   idx = int(std::round(t * float(this->values.size() - 1)));
  return std::clamp(idx, 0, int(this->values.size() - 1));
}

void VectorEditor::set_values(const std::vector<float> &v)
{
  this->values = v;
  this->update();

  Q_EMIT this->value_changed();
  Q_EMIT this->edit_ended();
}

QSize VectorEditor::sizeHint() const
{
  return {QSX_CONFIG->global.width_min,
          SINT(0.5f * SFLOAT(QSX_CONFIG->global.width_min))};
}

QRectF VectorEditor::value_area() const
{
  const int padding = QSX_CONFIG->global.padding;
  return this->rect().adjusted(padding, padding, -padding, -padding);
}

} // namespace qsx
