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
  float ratio = QSX_CONFIG->color_picker.preview_width_ratio;

  QPainter painter(this);
  QRectF   bar_rect(5, 3, ratio * SFLOAT(this->width()), this->height() - 6);

  // if hovered
  if (this->underMouse())
    painter.fillRect(this->rect(), QSX_CONFIG->global.color_hovered);

  // Gradient
  QLinearGradient grad(bar_rect.topLeft(), bar_rect.topRight());

  for (const auto &stop : this->stops)
    grad.setColorAt(stop.position, stop.color);

  painter.setBrush(grad);
  painter.setPen(Qt::gray);
  painter.drawRect(bar_rect);

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
