/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <algorithm>

#include <QColorDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QWidgetAction>

#include "qsx/color_gradient_picker.hpp"
#include "qsx/color_gradient_preview.hpp"
#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"

namespace qsx
{

ColorGradientPicker::ColorGradientPicker(const std::string &label_, QWidget *parent)
    : QWidget(parent), label(label_)
{
  this->setMinimumHeight(QSX_CONFIG->color_picker.height_min);

  // Default gradient
  this->stops = {{0.0, QColor(0, 0, 0, 255)}, {1.0, QColor(255, 255, 255, 255)}};
}

void ColorGradientPicker::contextMenuEvent(QContextMenuEvent *event)
{
  int index = this->find_stop_at_position(event->pos());

  if (index != -1)
  {
    QMenu    menu(this);
    QAction *remove_action = menu.addAction("Remove Stop");
    if (menu.exec(event->globalPos()) == remove_action)
    {
      if (this->stops.size() > 2)
      {
        this->stops.remove(index);
        this->update_gradient();

        Q_EMIT this->edit_ended();
      }
    }
  }
  else
  {
    this->show_presets_menu();
  }
}

void ColorGradientPicker::draw_checkerboard(QPainter &painter, const QRectF &rect) const
{
  return; // TODO DBG

  const int cell_size = 6;
  QColor    light(200, 200, 200);
  QColor    dark(150, 150, 150);

  for (int y = 0; y < rect.height(); y += cell_size)
    for (int x = 0; x < rect.width(); x += cell_size)
    {
      bool is_light = ((x / cell_size) + (y / cell_size)) % 2 == 0;
      painter.fillRect(SINT(rect.left()) + x,
                       SINT(rect.top()) + y,
                       cell_size,
                       cell_size,
                       is_light ? light : dark);
    }
}

int ColorGradientPicker::find_stop_at_position(const QPoint &pos) const
{
  for (int i = 0; i < this->stops.size(); ++i)
  {
    if (this->stop_rect(this->stops[i]).contains(pos))
      return i;
  }
  return -1;
}

QLinearGradient ColorGradientPicker::get_gradient() const
{
  QLinearGradient grad(0, 0, this->width(), 0);
  for (const auto &stop : this->stops)
    grad.setColorAt(stop.position, stop.color);
  return grad;
}

QVector<Stop> ColorGradientPicker::get_stops() const { return this->stops; }

void ColorGradientPicker::mouseDoubleClickEvent(QMouseEvent *event)
{
  int index = this->find_stop_at_position(event->pos());
  if (index != -1)
  {
    QColor new_color = QColorDialog::getColor(this->stops[index].color,
                                              this,
                                              "Select Color",
                                              QColorDialog::ShowAlphaChannel |
                                                  QColorDialog::DontUseNativeDialog);
    if (new_color.isValid())
    {
      this->stops[index].color = new_color;
      this->update_gradient();

      Q_EMIT this->edit_ended();
    }
  }
  else
  {
    QRectF bar_rect = this->rect().adjusted(10, 10, -10, -20);
    if (bar_rect.contains(event->pos()))
    {
      qreal pos = (event->pos().x() - bar_rect.left()) / bar_rect.width();
      this->stops.push_back({pos, QColor(255, 255, 255, 255)});
      this->sort_stops();
      this->selected_stop_index = this->find_stop_at_position(event->pos());
      this->update_gradient();

      Q_EMIT this->edit_ended();
    }
  }
}

void ColorGradientPicker::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    int index = this->find_stop_at_position(event->pos());
    if (index != -1)
    {
      this->selected_stop_index = index;
      this->dragging = true;
    }
  }
}

void ColorGradientPicker::mouseMoveEvent(QMouseEvent *event)
{
  if (this->dragging && this->selected_stop_index != -1)
  {
    QRectF bar_rect = this->rect().adjusted(10, 10, -10, -20);
    qreal  pos = (event->pos().x() - bar_rect.left()) / bar_rect.width();
    pos = std::clamp(pos, 0.0, 1.0);
    this->stops[this->selected_stop_index].position = pos;
    this->sort_stops();
    this->update_gradient();
  }
}

void ColorGradientPicker::mouseReleaseEvent(QMouseEvent *)
{
  // means end of dragging
  if (this->dragging)
    Q_EMIT this->edit_ended();

  this->dragging = false;
}

void ColorGradientPicker::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRectF bar_rect = this->rect().adjusted(10, 10, -10, -20);

  // Checkerboard background (for alpha visualization)
  this->draw_checkerboard(painter, bar_rect);

  // Draw gradient overlay
  QLinearGradient grad(bar_rect.topLeft(), bar_rect.topRight());
  for (const auto &stop : this->stops)
    grad.setColorAt(stop.position, stop.color);

  painter.setBrush(grad);
  painter.setPen(Qt::gray);
  painter.drawRoundedRect(bar_rect, QSX_CONFIG->global.radius, QSX_CONFIG->global.radius);

  // Draw color stops
  for (int i = 0; i < this->stops.size(); ++i)
  {
    QRectF r = this->stop_rect(this->stops[i]);
    this->draw_checkerboard(painter, r);
    painter.setBrush(this->stops[i].color);
    painter.setPen(i == this->selected_stop_index ? Qt::black : Qt::darkGray);
    painter.drawEllipse(r);
  }

  // label
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  QFontMetrics fm(this->font());
  int          base_dx = fm.horizontalAdvance(QString("M"));
  QRectF       rect_label = bar_rect.adjusted(base_dx, 0, -base_dx, 0);
  painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());
}

void ColorGradientPicker::set_gradient(const QLinearGradient &gradient)
{
  this->stops.clear();
  for (const auto &stop : gradient.stops())
  {
    this->stops.push_back({stop.first, stop.second});
  }
  this->sort_stops();
  this->update();
}

void ColorGradientPicker::set_presets(const std::vector<Preset> &new_presets)
{
  this->presets = new_presets;
}

void ColorGradientPicker::set_stops(const QVector<Stop> &new_stops)
{
  this->stops = new_stops;
  this->update();
}

void ColorGradientPicker::show_presets_menu()
{
  if (this->presets.empty())
    return;

  QMenu menu(this);

  for (auto &preset : this->presets)
  {
    QWidgetAction *action = new QWidgetAction(&menu);
    auto          *preview_widget = new GradientPreviewWidget(preset.name, preset.stops);
    action->setDefaultWidget(preview_widget);

    QObject::connect(preview_widget,
                     &GradientPreviewWidget::clicked,
                     this,
                     [this, &preset]()
                     {
                       this->stops = preset.stops;
                       this->sort_stops();
                       this->update_gradient();

                       Q_EMIT edit_ended();
                     });

    menu.addAction(action);
  }

  menu.exec(QCursor::pos());
}

QSize ColorGradientPicker::sizeHint() const
{
  return QSize(QSX_CONFIG->global.width_min, QSX_CONFIG->color_picker.height_min);
}

void ColorGradientPicker::sort_stops()
{
  std::sort(this->stops.begin(),
            this->stops.end(),
            [](const Stop &a, const Stop &b) { return a.position < b.position; });
}

QRectF ColorGradientPicker::stop_rect(const Stop &stop) const
{
  QRectF bar_rect = this->rect().adjusted(10, 10, -10, -20);
  qreal  x = bar_rect.left() + stop.position * bar_rect.width();
  qreal  y = bar_rect.bottom() + 5;
  return QRectF(x - 5, y, 10, 10);
}

void ColorGradientPicker::update_gradient()
{
  Q_EMIT this->value_changed();
  Q_EMIT this->gradient_changed(this->get_gradient());
  this->update();
}

} // namespace qsx
