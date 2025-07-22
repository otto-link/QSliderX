/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <format>

#include <QEvent>
#include <QHoverEvent>
#include <QPainter>

#include "qsx/config.hpp"
#include "qsx/internal/interpolate1d.hpp"
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
    this->is_onoff_hovered = false;
    this->is_reset_hovered = false;
    this->is_reset_unit_hovered = false;
    this->is_autorange_hovered = false;
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
    this->is_onoff_hovered = this->rect_onoff.contains(pos);
    this->is_reset_hovered = this->rect_reset.contains(pos);
    this->is_reset_unit_hovered = this->rect_reset_unit.contains(pos);
    this->is_autorange_hovered = this->rect_autorange.contains(pos);
    this->update();
  }
  break;

  default:
    break;
  }

  return QWidget::event(event);
}

void SliderRange::force_values(float new_value0, float new_value1)
{
  bool check_reversed_range = false;
  bool ret = this->set_value(0, new_value0, check_reversed_range);
  ret |= this->set_value(1, new_value1, check_reversed_range);

  if (ret)
    Q_EMIT this->value_has_changed();
}

bool SliderRange::get_is_enabled() const { return this->is_enabled; }

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

    event->accept();
  }

  QWidget::mouseMoveEvent(event);
}

void SliderRange::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->is_onoff_hovered)
    {
      this->set_is_enabled(!this->get_is_enabled());
    }

    if (this->is_enabled)
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
      else if (this->is_reset_hovered)
      {
        this->force_values(this->value0_init, this->value1_init);
      }
      else if (this->is_reset_unit_hovered)
      {
        this->force_values(0.f, 1.f);
      }
      else if (this->is_autorange_hovered)
      {
        this->set_autorange(!this->autorange);
      }
    }
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void SliderRange::mouseReleaseEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    this->set_is_dragging(false);

    if (this->get_value(this->dragged_value_id) != this->value_before_dragging)
      Q_EMIT this->value_has_changed();
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void SliderRange::on_update_bins()
{
  if (this->histogram_fct)
  {
    this->bins = this->histogram_fct();

    if (this->bins.first.size() && this->autorange)
    {
      // retrieve min/max from histogram
      this->vmin = *std::min_element(bins.first.begin(), bins.first.end());
      this->vmax = *std::max_element(bins.first.begin(), bins.first.end());

      // but still makes sure current values are within the range
      this->vmin = std::min(this->value0, this->vmin);
      this->vmax = std::max(this->value1, this->vmax);

      this->force_values(this->value0, this->value1);
      this->update_value_positions();
      this->update();
    }
  }
  else
  {
    this->bins.first.clear();
    this->bins.second.clear();
  }
}

void SliderRange::paintEvent(QPaintEvent *)
{
  const int radius = QSX_CONFIG->global.radius;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect_bar, radius, radius);

  if (this->is_enabled)
  {

    // value bar
    if (bins.first.size() && bins.first.size() == bins.second.size())
    {
      // bins normalization
      float bmax = *std::max_element(bins.second.begin(), bins.second.end());

      // interpolate bins data
      Interpolator1D fitp = Interpolator1D(bins.first,
                                           bins.second,
                                           InterpolationMethod1D::AKIMA);

      // used to avoid extrapolation
      float xmin = bins.first.front();
      float xmax = bins.first.back();

      // draw...
      int   nn = this->rect_bar.width(); // one per pixel
      float dr = 1.f / static_cast<float>(nn - 1);
      int   p0 = this->rect_handle_min.center().x();
      int   p1 = this->rect_handle_max.center().x();
      int   gap = radius;

      for (int k = 0; k < nn - 1; ++k)
      {
        float r0 = static_cast<float>(k) * dr;
        float r1 = static_cast<float>(k + 1) * dr;

        // value
        float v0 = r0 * (this->vmax - this->vmin) + this->vmin;
        float v1 = r1 * (this->vmax - this->vmin) + this->vmin;

        float factor = 0.9f;
        float y0 = (v0 >= xmin && v0 <= xmax) ? factor * fitp(v0) / bmax : 0.f;
        float y1 = (v1 >= xmin && v1 <= xmax) ? factor * fitp(v1) / bmax : 0.f;

        y0 = std::clamp(y0, 0.f, 1.f);
        y1 = std::clamp(y1, 0.f, 1.f);

        // draw positions (add margin beginning/end)
        float lx = static_cast<float>(this->rect_bar.width() - 2 * gap);
        int   pos0 = gap + static_cast<int>(r0 * lx);
        int   pos1 = gap + static_cast<int>(r1 * lx);

        int dy0 = static_cast<int>(static_cast<float>(this->rect_bar.height()) *
                                   (1.f - y0));
        int dy1 = static_cast<int>(static_cast<float>(this->rect_bar.height()) *
                                   (1.f - y1));

        painter.setPen(Qt::NoPen);
        if (pos0 >= p0 && pos1 <= p1)
        {
          painter.setBrush(QSX_CONFIG->global.color_selected);
        }
        else
          painter.setBrush(QSX_CONFIG->global.color_faded);

        const QPoint points[4] = {
            QPoint(pos0, dy0 + 1),
            QPoint(pos1, dy1 + 1),
            QPoint(pos1, this->rect_bar.height() - 1),
            QPoint(pos0, this->rect_bar.height() - 1),
        };

        painter.drawPolygon(points, 4);
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

    // values
    painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
    painter.setPen(QPen(QSX_CONFIG->global.color_text));

    painter.drawText(this->rect_handle_min.center() + QPoint(0, this->base_dy),
                     this->get_value_as_string(0).c_str());
    painter.drawText(this->rect_handle_max.center() + QPoint(0, this->base_dy),
                     this->get_value_as_string(1).c_str());
  }

  // labels
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  painter.drawText(this->rect_label,
                   Qt::AlignLeft | Qt::AlignVCenter,
                   this->label.c_str());

  painter.drawText(this->rect_reset,
                   Qt::AlignCenter | Qt::AlignVCenter,
                   QString::fromUtf8(u8"R"));

  painter.drawText(this->rect_reset_unit,
                   Qt::AlignCenter | Qt::AlignVCenter,
                   QString::fromUtf8(u8"-"));

  if (this->autorange)
    painter.setPen(QSX_CONFIG->global.color_selected);
  else
    painter.setPen(QPen(QSX_CONFIG->global.color_text));

  painter.drawText(this->rect_autorange,
                   Qt::AlignCenter | Qt::AlignVCenter,
                   QString::fromUtf8(u8"A"));

  if (this->is_enabled)
    painter.setPen(QSX_CONFIG->global.color_selected);
  else
    painter.setPen(QPen(QSX_CONFIG->global.color_text));

  painter.drawText(this->rect_onoff,
                   Qt::AlignCenter | Qt::AlignVCenter,
                   QString::fromUtf8(u8"●"));

  // buttons border
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  if (this->is_onoff_hovered)
    painter.drawRect(this->rect_onoff.adjusted(0, 2, 0, -2));
  else if (this->is_reset_hovered)
    painter.drawRect(this->rect_reset.adjusted(0, 2, 0, -2));
  else if (this->is_reset_unit_hovered)
    painter.drawRect(this->rect_reset_unit.adjusted(0, 2, 0, -2));
  else if (this->is_autorange_hovered)
    painter.drawRect(this->rect_autorange.adjusted(0, 2, 0, -2));
}

void SliderRange::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void SliderRange::set_histogram_fct(std::function<PairVec()> new_histogram_fct)
{
  this->histogram_fct = new_histogram_fct;
  this->on_update_bins();
  this->update();
}

QSize SliderRange::sizeHint() const
{
  return QSize(this->slider_width, this->slider_height);
}

void SliderRange::set_autorange(bool new_state)
{
  this->autorange = new_state;
  this->on_update_bins();
}

void SliderRange::set_is_dragging(bool new_state)
{
  this->is_dragging = new_state;

  if (this->is_dragging)
    this->setCursor(Qt::SizeHorCursor);
  else
    this->setCursor(Qt::ArrowCursor);
}

void SliderRange::set_is_enabled(bool new_state)
{
  this->is_enabled = new_state;
  this->update();
  Q_EMIT this->value_has_changed();
}

bool SliderRange::set_value(int id, float new_value, bool check_reversed_range)
{
  float cmin = this->vmin;
  float cmax = this->vmax;

  // ensure min value is always below max value...
  if (check_reversed_range)
  {
    cmin = id == 0 ? this->vmin : this->value0;
    cmax = id == 0 ? this->value1 : this->vmax;
  }

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

void SliderRange::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = fm.horizontalAdvance(QString("M"));
  this->base_dy = fm.height() + QSX_CONFIG->slider.padding_v;

  // TODO fix
  int label_width = 2 * this->base_dx + fm.horizontalAdvance(this->label.c_str());
  int buttons_width = 4 * this->base_dx;

  this->slider_width = buttons_width + 2 * label_width;
  this->slider_height = static_cast<int>(2.25f * static_cast<float>(this->base_dy));

  // size
  this->setMinimumWidth(this->slider_width);
  this->setMinimumHeight(this->slider_height);
  this->setMaximumHeight(this->slider_height + this->base_dy);

  // rectangles
  int   base_dx_half = static_cast<int>(0.5f * static_cast<float>(this->base_dx));
  QSize bsize = QSize(this->base_dx + base_dx_half, this->base_dy); // buttons size

  this->rect_reset_unit = QRect(
      QPoint(this->rect().width() - base_dx_half - 4 * bsize.width(), 0),
      bsize);
  this->rect_autorange = QRect(
      QPoint(this->rect().width() - base_dx_half - 3 * bsize.width(), 0),
      bsize);
  this->rect_reset = QRect(
      QPoint(this->rect().width() - base_dx_half - 2 * bsize.width(), 0),
      bsize);
  this->rect_onoff = QRect(
      QPoint(this->rect().width() - base_dx_half - 1 * bsize.width(), 0),
      bsize);

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
