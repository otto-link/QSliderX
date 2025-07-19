/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneHoverEvent>
#include <QHoverEvent>
#include <QPainter>

#include "qsx/config.hpp"
#include "qsx/logger.hpp"
#include "qsx/slider_int.hpp"
#include "qsx/utils.hpp"

namespace qsx
{

SliderInt::SliderInt(const std::string &label_,
                     int                value_init_,
                     int                vmin_,
                     int                vmax_,
                     bool               add_plus_minus_buttons_,
                     QWidget           *parent)
    : QWidget(parent), value_init(value_init_), value(value_init_), vmin(vmin_),
      vmax(vmax_), add_plus_minus_buttons(add_plus_minus_buttons_)
{
  QSXLOG->trace("SliderInt::SliderInt");

  this->label = truncate_string(label_,
                                static_cast<size_t>(QSX_CONFIG->global.max_label_len));

  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);

  this->update_geometry();
}

bool SliderInt::event(QEvent *event)
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
    this->is_minus_hovered = false;
    this->is_plus_hovered = false;
    this->is_bar_hovered = false;
    this->setCursor(Qt::ArrowCursor);
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    auto  *hover = static_cast<QHoverEvent *>(event);
    QPoint pos = hover->position().toPoint(); // mouse position inside the widget

    this->is_minus_hovered = this->rect_minus.contains(pos);
    this->is_plus_hovered = this->rect_plus.contains(pos);
    this->is_bar_hovered = this->rect_bar.contains(pos);

    if (this->is_bar_hovered)
      this->setCursor(Qt::SizeHorCursor);
    else
      this->setCursor(Qt::ArrowCursor);

    this->update();
  }
  break;

  default:
    break;
  }
  return QWidget::event(event); // Call base class implementation
}

int SliderInt::get_value() const { return this->value; }

void SliderInt::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    // pixels per unit
    float ppu = QSX_CONFIG->slider.ppu;

    if (event->modifiers() & Qt::ControlModifier)
      ppu *= QSX_CONFIG->slider.ppu_multiplier_fine_tuning;

    int dx = event->position().toPoint().x() - this->pos_x_before_dragging;
    int dv = static_cast<int>(static_cast<float>(dx) / ppu);
    int new_value = std::clamp(this->value_before_dragging + dv, this->vmin, this->vmax);
    this->set_value(new_value);
  }

  QWidget::mouseMoveEvent(event);
}

void SliderInt::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton && this->is_bar_hovered)
  {
    this->value_before_dragging = this->value;
    this->pos_x_before_dragging = event->position().toPoint().x();
    this->is_dragging = true;
  }

  QWidget::mousePressEvent(event);
}

void SliderInt::mouseReleaseEvent(QMouseEvent *event)
{
  this->is_dragging = false;
  if (this->value != this->value_before_dragging)
    Q_EMIT this->value_has_changed();

  QWidget::mouseReleaseEvent(event);
}

void SliderInt::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(),
                          QSX_CONFIG->global.radius,
                          QSX_CONFIG->global.radius);

  // value bar
  if (this->vmin != -INT_MAX && this->vmax != INT_MAX)
  {
    painter.setBrush(QBrush(QSX_CONFIG->global.color_selected));
    painter.setPen(Qt::NoPen);

    float r = static_cast<float>(this->value - this->vmin) /
              static_cast<float>(this->vmax - this->vmin);
    int rcut = static_cast<int>((1.f - r) * static_cast<float>(this->rect_bar.width()));

    if (this->add_plus_minus_buttons)
    {
      painter.drawRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1));
    }
    else
    {
      painter.drawRoundedRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1),
                              QSX_CONFIG->global.radius,
                              QSX_CONFIG->global.radius);
    }
  }

  // labels
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  QRect rect_label = this->rect_bar.adjusted(this->base_dx, 0, -this->base_dx, 0);
  painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());

  painter.drawText(rect_label,
                   Qt::AlignRight | Qt::AlignVCenter,
                   std::to_string(this->value).c_str());

  // arrows
  QString left = this->is_minus_hovered ? "◀" : "◁";
  QString right = this->is_plus_hovered ? "▶" : "▷";

  painter.drawText(this->rect_minus, Qt::AlignCenter | Qt::AlignVCenter, left);
  painter.drawText(this->rect_plus, Qt::AlignCenter | Qt::AlignVCenter, right);
}

void SliderInt::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void SliderInt::set_value(int new_value)
{
  if (new_value != this->value)
  {
    this->value = new_value;
    this->update();
    Q_EMIT this->value_changed();
  }
}

QSize SliderInt::sizeHint() const { return QSize(this->slider_width_min, this->base_dy); }

void SliderInt::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = fm.horizontalAdvance(QString("M"));
  this->base_dy = fm.height() + QSX_CONFIG->slider.padding_v;

  // TODO slider width
  int label_width = QSX_CONFIG->global.max_label_len * this->base_dx;
  this->slider_width_min = label_width + 64;

  // rectangles
  if (this->add_plus_minus_buttons)
  {
    this->rect_minus = this->rect();
    this->rect_minus.setWidth(2 * this->base_dx);

    this->rect_plus = this->rect().adjusted(this->rect().width() - 2 * this->base_dx,
                                            0,
                                            0,
                                            0);
  }
  else
  {
    this->rect_minus = QRect();
    this->rect_plus = QRect();
  }

  int gap = this->add_plus_minus_buttons ? 2 * this->base_dx : 0;
  this->rect_bar = this->rect().adjusted(gap, 0, -gap, 0);

  // size
  this->setMinimumWidth(this->sizeHint().width());
  this->setMinimumHeight(this->sizeHint().height());
  this->setMaximumHeight(this->sizeHint().height());
}

} // namespace qsx
