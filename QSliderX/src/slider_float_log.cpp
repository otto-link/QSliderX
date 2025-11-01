/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <cfloat>
#include <cmath>

#include <QMouseEvent>
#include <QPainter>

#include "qsx/config.hpp"
#include "qsx/internal/utils.hpp"
#include "qsx/slider_float_log.hpp"

namespace qsx
{

SliderFloatLog::SliderFloatLog(const std::string &label_,
                               float              value_init_,
                               float              vmin_,
                               float              vmax_,
                               bool               add_plus_minus_buttons_,
                               const std::string &value_format_,
                               QWidget           *parent)
    : SliderFloat(label_,
                  value_init_,
                  vmin_,
                  vmax_,
                  add_plus_minus_buttons_,
                  value_format_,
                  parent)
{
  // require strictly positive range
  if (vmin_ <= 0.f || vmax_ <= 0.f)
    throw std::runtime_error("SliderFloatLog requires positive min and max values.");

  this->log_min = std::log10(vmin_);
  this->log_max = std::log10(vmax_);
}

float SliderFloatLog::from_log(float lv) const { return std::pow(10.f, lv); }

void SliderFloatLog::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    float ppu = SFLOAT(this->rect_bar.width()) / (this->log_max - this->log_min);

    if (event->modifiers() & Qt::ControlModifier)
      ppu *= QSX_CONFIG->slider.ppu_multiplier_fine_tuning;
    else if (event->modifiers() & Qt::ShiftModifier)
      ppu /= QSX_CONFIG->slider.ppu_multiplier_fine_tuning;

    int   dx = event->position().toPoint().x() - this->pos_x_before_dragging;
    float dlogv = SFLOAT(dx) / ppu;

    float current_log = to_log(this->value_before_dragging);
    float new_log = std::clamp(current_log + dlogv, this->log_min, this->log_max);

    this->set_value(from_log(new_log));
  }

  QWidget::mouseMoveEvent(event);
}

void SliderFloatLog::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    bool  is_range_limited = this->vmin != -FLT_MAX && this->vmax != FLT_MAX;
    float delta_log = is_range_limited ? (this->log_max - this->log_min) /
                                             QSX_CONFIG->slider.button_ticks
                                       : 0.1f;

    if (this->is_bar_hovered)
    {
      this->value_before_dragging = this->value;
      this->pos_x_before_dragging = event->position().toPoint().x();
      this->set_is_dragging(true);
    }
    else if (this->is_minus_hovered)
    {
      float new_log = std::log10(this->get_value()) - delta_log;
      if (this->set_value(from_log(new_log)))
        Q_EMIT this->edit_ended();
    }
    else if (this->is_plus_hovered)
    {
      float new_log = std::log10(this->get_value()) + delta_log;
      if (this->set_value(from_log(new_log)))
        Q_EMIT this->edit_ended();
    }
  }
  else if (event->button() == Qt::RightButton)
  {
    this->show_context_menu();
  }
}

void SliderFloatLog::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(),
                          QSX_CONFIG->global.radius,
                          QSX_CONFIG->global.radius);

  // value bar (logarithmic position)
  if (this->vmin != -FLT_MAX && this->vmax != FLT_MAX && !this->value_edit->isVisible())
  {
    const float range = this->log_max - this->log_min;
    const float r = (to_log(this->value) - this->log_min) / range;

    if (r > 0.f)
    {
      const int rcut = SINT((1.f - r) * SFLOAT(this->rect_bar.width()));

      painter.setBrush(QSX_CONFIG->global.color_selected);
      painter.setPen(Qt::NoPen);

      if (this->add_plus_minus_buttons)
        painter.drawRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1));
      else
        painter.drawRoundedRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1),
                                QSX_CONFIG->global.radius,
                                QSX_CONFIG->global.radius);
    }
  }

  // label + value
  painter.setPen(QPen(QSX_CONFIG->global.color_text));
  QRect rect_label = this->rect_bar.adjusted(this->base_dx, 0, -this->base_dx, 0);
  painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());
  painter.drawText(rect_label,
                   Qt::AlignRight | Qt::AlignVCenter,
                   this->get_value_as_string().c_str());

  // arrows
  QString left = this->is_minus_hovered ? "◀" : "◁";
  QString right = this->is_plus_hovered ? "▶" : "▷";

  painter.drawText(this->rect_minus, Qt::AlignCenter | Qt::AlignVCenter, left);
  painter.drawText(this->rect_plus, Qt::AlignCenter | Qt::AlignVCenter, right);
}

bool SliderFloatLog::set_value(float new_value)
{
  new_value = std::clamp(new_value, this->vmin, this->vmax);

  if (new_value == this->value)
    return false;

  this->value = new_value;
  this->update();
  Q_EMIT this->value_changed();
  return true;
}

float SliderFloatLog::to_log(float v) const
{
  return std::log10(std::clamp(v, vmin, vmax));
}

} // namespace qsx
