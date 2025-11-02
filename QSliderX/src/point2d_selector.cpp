/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <algorithm>
#include <format>

#include <QMouseEvent>
#include <QPainter>

#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"
#include "qsx/point2d_selector.hpp"

namespace qsx
{

Point2DSelector::Point2DSelector(const std::string &label_,
                                 float              xmin_,
                                 float              xmax_,
                                 float              ymin_,
                                 float              ymax_,
                                 QWidget           *parent)
    : QWidget(parent), label(label_), xmin(xmin_), xmax(xmax_), ymin(ymin_), ymax(ymax_),
      value{(xmin_ + xmax_) * 0.5f, (ymin_ + ymax_) * 0.5f}
{
  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
}

bool Point2DSelector::event(QEvent *event)
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
    this->is_point_hovered = false;
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    auto  *hover = static_cast<QHoverEvent *>(event);
    QPoint mouse_pos = hover->position().toPoint(); // mouse position inside the widget

    if (!this->is_dragging)
    {
      int     radius = QSX_CONFIG->canvas.point_radius;
      QPointF pos = this->map_to_widget(this->value);
      QRect   prect = QRect(QPoint(SINT(pos.x()), SINT(pos.y())) - QPoint(radius, radius),
                          QSize(2 * radius, 2 * radius));

      this->is_point_hovered = prect.contains(mouse_pos);
    }

    this->update();
  }
  break;

  default:
    break;
  }

  return QWidget::event(event);
}

std::pair<float, float> Point2DSelector::get_value() const { return this->value; }

QPointF Point2DSelector::map_to_widget(const std::pair<float, float> &v) const
{
  const int padding = QSX_CONFIG->global.padding;
  QRectF    area = this->rect().adjusted(padding, padding, -padding, -padding);
  float     nx = (v.first - this->xmin) / (this->xmax - this->xmin);
  float     ny = (v.second - this->ymin) / (this->ymax - this->ymin);
  return QPointF(area.left() + nx * area.width(),
                 area.bottom() - ny * area.height()); // invert Y
}

std::pair<float, float> Point2DSelector::map_from_widget(const QPoint &p) const
{
  const int padding = QSX_CONFIG->global.padding;
  QRectF    area = this->rect().adjusted(padding, padding, -padding, -padding);
  float     nx = SFLOAT((p.x() - area.left()) / area.width());
  float     ny = SFLOAT(1.0f - (p.y() - area.top()) / area.height());
  return {this->xmin + nx * (this->xmax - this->xmin),
          this->ymin + ny * (this->ymax - this->ymin)};
}

void Point2DSelector::mousePressEvent(QMouseEvent *event)
{
  QPointF p = this->map_to_widget(this->value);
  if (QLineF(event->pos(), p).length() < 10.0)
    this->is_dragging = true;
}

void Point2DSelector::mouseMoveEvent(QMouseEvent *event)
{
  if (!this->is_dragging)
    return;

  const int padding = QSX_CONFIG->global.padding;
  QRectF    area = this->rect().adjusted(padding, padding, -padding, -padding);
  QPoint    clamped(std::clamp(event->pos().x(),
                            static_cast<int>(area.left()),
                            static_cast<int>(area.right())),
                 std::clamp(event->pos().y(),
                            static_cast<int>(area.top()),
                            static_cast<int>(area.bottom())));

  auto new_val = this->map_from_widget(clamped);
  this->set_value(new_val);
  Q_EMIT this->value_changed();
}

void Point2DSelector::mouseReleaseEvent(QMouseEvent * /*event*/)
{
  if (this->is_dragging)
    Q_EMIT this->edit_ended();

  this->is_dragging = false;
}

void Point2DSelector::paintEvent(QPaintEvent *)
{
  const int radius = QSX_CONFIG->global.radius;
  const int padding = QSX_CONFIG->global.padding;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(), radius, radius);

  QRectF area = this->rect().adjusted(padding, padding, -padding, -padding);

  // value label

  if (QSX_CONFIG->point2d.show_value)
  {
    std::string value_str = std::format("({:.3f}, {:.3f})",
                                        this->value.first,
                                        this->value.second);

    QFontMetrics fm(this->font());
    int          base_dx = text_width(this, "M");

    QRect rect_label = QRect(
        QPoint(base_dx, 0),
        QSize(this->rect().width() - base_dx,
              this->rect().height() - 2 * QSX_CONFIG->global.padding));

    painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
    painter.setPen(QPen(QSX_CONFIG->global.color_text));
    painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignBottom, value_str.c_str());
  }

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

  // draw bounding box
  painter.setPen(QSX_CONFIG->global.color_border);
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(area);

  // draw crosshair (if 0 is inside range)
  painter.setPen(QPen(QSX_CONFIG->global.color_faded, 1, Qt::DashLine));
  if (this->xmin < 0 && this->xmax > 0)
  {
    float nx = (0.0f - this->xmin) / (this->xmax - this->xmin);
    float x = SFLOAT(area.left() + nx * area.width());
    painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
  }
  if (this->ymin < 0 && this->ymax > 0)
  {
    float ny = (0.0f - this->ymin) / (this->ymax - this->ymin);
    float y = SFLOAT(area.bottom() - ny * area.height());
    painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
  }

  // draw selection point
  QPointF p = this->map_to_widget(this->value);
  painter.setPen(QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));
  painter.setBrush(this->is_point_hovered ? QBrush(QSX_CONFIG->global.color_selected)
                                          : QBrush(QSX_CONFIG->global.color_bg));
  painter.drawEllipse(p,
                      QSX_CONFIG->canvas.point_radius,
                      QSX_CONFIG->canvas.point_radius);
}

void Point2DSelector::resizeEvent(QResizeEvent *) { this->update(); }

void Point2DSelector::set_value(const std::pair<float, float> &v)
{
  this->value.first = std::clamp(v.first, this->xmin, this->xmax);
  this->value.second = std::clamp(v.second, this->ymin, this->ymax);
  this->update();
}

QSize Point2DSelector::sizeHint() const
{
  return {QSX_CONFIG->global.width_min,
          SINT(0.5f * SFLOAT(QSX_CONFIG->global.width_min))};
}

} // namespace qsx
