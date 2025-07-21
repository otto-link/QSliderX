/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <format>

#include <QEvent>
#include <QHoverEvent>
#include <QPainter>

#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"
#include "qsx/slider_range.hpp"

namespace qsx
{

SliderRange::SliderRange(const std::string &label_,
                         float              value0_init_,
                         float              value1_init_,
                         float              vmin_,
                         float              vmax_,
                         const std::string &value_format_,
                         QWidget           *parent)
    : QWidget(parent), value0_init(value0_init_), value1_init(value1_init_),
      value0(value0_init_), value1(value1_init_), vmin(vmin_), vmax(vmax_),
      value_format(value_format_)
{
  QSXLOG->trace("SliderRange::SliderRange");

  this->label = truncate_string(label_,
                                static_cast<size_t>(QSX_CONFIG->global.max_label_len));

  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->update_geometry();
}

bool SliderRange::event(QEvent *event)
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
    this->is_min_hovered = false;
    this->is_max_hovered = false;
    this->is_range_hovered = false;
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    auto  *hover = static_cast<QHoverEvent *>(event);
    QPoint pos = hover->position().toPoint(); // mouse position inside the widget

    this->is_min_hovered = this->rect_handle_min.adjusted(-1, -1, 1, 1).contains(pos);
    this->is_max_hovered = this->rect_handle_max.adjusted(-1, -1, 1, 1).contains(pos);
    this->is_range_hovered = this->rect_range.contains(pos);
    this->update();
  }
  break;

  default:
    break;
  }
  return QWidget::event(event);
}

float SliderRange::get_value(int id) const
{
  if (id == 0)
    return this->value0;
  else
    return this->value1;
}

std::string SliderRange::get_value_as_string(int id) const
{
  const float v = this->get_value(id);
  return std::vformat(this->value_format, std::make_format_args(v));
}

float SliderRange::get_vmax() const { return this->vmax; }

float SliderRange::get_vmin() const { return this->vmin; }

void SliderRange::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    // pixels per unit
    float ppu = this->vmax != this->vmin ? static_cast<float>(this->rect_bar.width()) /
                                               (this->vmax - this->vmin)
                                         : 1.f;

    if (event->modifiers() & Qt::ControlModifier)
      ppu *= QSX_CONFIG->slider.ppu_multiplier_fine_tuning;

    int   dx = event->position().toPoint().x() - this->pos_x_before_dragging;
    float dv = static_cast<float>(dx) / ppu;
    this->set_value(this->dragged_value_id, this->value_before_dragging + dv);
  }

  QWidget::mouseMoveEvent(event);
}

void SliderRange::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->is_min_hovered)
    {
      this->dragged_value_id = 0;
      this->value_before_dragging = this->value0;
      this->pos_x_before_dragging = event->position().toPoint().x();
      this->set_is_dragging(true);
    }
    else if (this->is_max_hovered)
    {
      this->dragged_value_id = 1;
      this->value_before_dragging = this->value1;
      this->pos_x_before_dragging = event->position().toPoint().x();
      this->set_is_dragging(true);
    }
  }

  QWidget::mousePressEvent(event);
}

void SliderRange::mouseReleaseEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    this->set_is_dragging(false);

    if (this->get_value(this->dragged_value_id) != this->value_before_dragging)
      Q_EMIT this->value_has_changed();
  }

  QWidget::mouseReleaseEvent(event);
}

void SliderRange::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  //
  painter.drawRoundedRect(this->rect_bar,
                          QSX_CONFIG->global.radius,
                          QSX_CONFIG->global.radius);

  // value bar
  if (bins.first.size() && bins.first.size() == bins.second.size())
  {
    // normalized
    float bmax = *std::max_element(bins.second.begin(), bins.second.end());

    const int p0 = this->rect_handle_min.center().x();
    const int p1 = this->rect_handle_max.center().x();

    for (size_t k = 0; k < bins.second.size(); ++k)
    {
      float v = 0.9f * bins.second[k] / bmax;
      int   pv0 = static_cast<int>(static_cast<float>(this->rect_bar.width()) *
                                 static_cast<float>(k) /
                                 static_cast<float>(bins.second.size()));
      int   pv1 = static_cast<int>(static_cast<float>(this->rect_bar.width()) *
                                 static_cast<float>(k + 1) /
                                 static_cast<float>(bins.second.size()));
      int dy = static_cast<int>(static_cast<float>(this->rect_bar.height()) * (1.f - v));

      painter.setPen(Qt::NoPen);
      if (pv0 >= p0 && pv1 <= p1)
      {
        painter.setBrush(QSX_CONFIG->global.color_selected);
      }
      else
        painter.setBrush(QSX_CONFIG->global.color_faded);

      if (k == 0)
        pv0 += QSX_CONFIG->global.radius;
      else if (k == bins.second.size() - 1)
        pv1 -= QSX_CONFIG->global.radius;

      painter.drawRect(
          QRect(QPoint(pv0, dy + 1), QPoint(pv1, this->rect_bar.height() - 1)));
    }
  }
  else
  {
    painter.setBrush(QSX_CONFIG->global.color_selected);
    painter.setPen(Qt::NoPen);
    painter.drawRect(this->rect_range.adjusted(1, 1, -1, -1));
  }

  // handles
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));

  painter.setPen(
      this->is_min_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));
  painter.drawEllipse(this->rect_handle_min);

  painter.setPen(
      this->is_max_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));
  painter.drawEllipse(this->rect_handle_max);

  // labels
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  painter.drawText(this->rect_label,
                   Qt::AlignLeft | Qt::AlignVCenter,
                   this->label.c_str());

  // painter.drawText(rect_label, Qt::AlignRight | Qt::AlignVCenter, "|+-R");

  // values
  painter.drawText(this->rect_handle_min.center() + QPoint(0, this->base_dy),
                   this->get_value_as_string(0).c_str());
  painter.drawText(this->rect_handle_max.center() + QPoint(0, this->base_dy),
                   this->get_value_as_string(1).c_str());
}

void SliderRange::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void SliderRange::set_histogram_fct(std::function<PairVec()> new_histogram_fct)
{
  this->histogram_fct = new_histogram_fct;
  this->update_bins();
  this->update();
}

QSize SliderRange::sizeHint() const
{
  return QSize(this->slider_width, this->slider_height);
}

void SliderRange::set_is_dragging(bool new_state)
{
  this->is_dragging = new_state;

  if (this->is_dragging)
    this->setCursor(Qt::SizeHorCursor);
  else
    this->setCursor(Qt::ArrowCursor);
}

bool SliderRange::set_value(int id, float new_value)
{
  // ensure min value is always below max value...
  float cmin = id == 0 ? this->vmin : this->value0;
  float cmax = id == 0 ? this->value1 : this->vmax;

  new_value = std::clamp(new_value, cmin, cmax);

  float *p_value = id == 0 ? &this->value0 : &this->value1;

  if (new_value == *p_value)
  {
    return false;
  }
  else
  {
    *p_value = new_value;
    this->update_value_positions();
    this->update();
    Q_EMIT this->value_changed();
  }

  return true;
}

void SliderRange::update_bins()
{
  if (this->histogram_fct)
    this->bins = this->histogram_fct();
  else
  {
    this->bins.first.clear();
    this->bins.second.clear();
  }
}

void SliderRange::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = fm.horizontalAdvance(QString("M"));
  this->base_dy = fm.height() + QSX_CONFIG->slider.padding_v;

  // TODO fix
  this->slider_width = 256;
  this->slider_width_min = 256;
  this->slider_height = 3 * this->base_dy;

  // size
  this->setMinimumWidth(this->slider_width_min);
  this->setMinimumHeight(this->slider_height);
  this->setMaximumHeight(this->slider_height);

  // rectangles
  this->rect_bar = this->rect().adjusted(0, 0, 0, -this->base_dy);
  this->rect_label = QRect(QPoint(this->base_dx, 0),
                           QSize(this->rect_bar.width() - this->base_dx, this->base_dy));

  this->update_value_positions();
}

void SliderRange::update_value_positions()
{
  // value positions
  const float range = this->vmax - this->vmin;
  const float r0 = range ? (this->value0 - this->vmin) / range : 0.f;
  const float r1 = range ? (this->value1 - this->vmin) / range : 1.f;
  const int   rpos0 = static_cast<int>(r0 * static_cast<float>(this->rect_bar.width()));
  const int   rpos1 = static_cast<int>(r1 * static_cast<float>(this->rect_bar.width()));

  const int dr = QSX_CONFIG->range.handle_radius;

  this->rect_handle_min = QRect(QPoint(rpos0 - dr, this->rect_bar.bottom() - dr),
                                QPoint(rpos0 + dr, this->rect_bar.bottom() + dr));

  this->rect_handle_max = QRect(QPoint(rpos1 - dr, this->rect_bar.bottom() - dr),
                                QPoint(rpos1 + dr, this->rect_bar.bottom() + dr));

  this->rect_range = this->rect_bar.adjusted(rpos0, 0, rpos1 - this->rect_bar.width(), 0);
}

} // namespace qsx
