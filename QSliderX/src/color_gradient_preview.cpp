/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include "qsx/color_gradient_preview.hpp"
#include "qsx/color_gradient_picker.hpp"
#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"

namespace qsx
{

GradientPreviewWidget::GradientPreviewWidget(const QString       &name_,
                                             const QVector<Stop> &stops_,
                                             QWidget             *parent)
    : QWidget(parent), name(name_), stops(stops_)
{
  this->setFixedSize(160, 26);
  this->setMouseTracking(true);
}

void GradientPreviewWidget::mousePressEvent(QMouseEvent *) { Q_EMIT this->clicked(); }

void GradientPreviewWidget::paintEvent(QPaintEvent *)
{
  float ratio = 0.2f;

  QPainter painter(this);
  QRectF   bar_rect(5, 3, ratio * SFLOAT(this->width()), this->height() - 6);

  // // Checkerboard
  // int    checker_size = 6;
  // QColor c1(200, 200, 200);
  // QColor c2(255, 255, 255);
  // for (int y = 0; y < bar_rect.height(); y += checker_size)
  //   for (int x = 0; x < bar_rect.width(); x += checker_size)
  //   {
  //     painter.fillRect(
  //         QRectF(bar_rect.left() + x, bar_rect.top() + y, checker_size, checker_size),
  //         ((x / checker_size + y / checker_size) % 2 == 0) ? c1 : c2);
  //   }

  // Gradient
  QLinearGradient grad(bar_rect.topLeft(), bar_rect.topRight());

  for (const auto &stop : this->stops)
    grad.setColorAt(stop.position, stop.color);

  painter.setBrush(grad);
  painter.setPen(Qt::gray);
  painter.drawRect(bar_rect);

  if (this->underMouse())
  {
    painter.fillRect(this->rect(), QColor(0, 0, 0, 20));
  }

  // Label
  painter.setPen(Qt::black);
  painter.drawText(QRectF(ratio * SFLOAT(this->width()) + 10,
                          0,
                          (1.f - ratio) * SFLOAT(this->width()) - 10,
                          this->height()),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   name);
}

} // namespace qsx
