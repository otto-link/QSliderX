/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

#include "qsx/color_picker.hpp"
#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"

namespace qsx
{

ColorPicker::ColorPicker(const std::string &label_, QWidget *parent)
    : QWidget(parent), color(Qt::gray), label(label_)
{
  this->setMinimumHeight(QSX_CONFIG->color_picker.height_min);
  this->setCursor(Qt::PointingHandCursor);
}

void ColorPicker::set_color(const QColor &new_color)
{
  if (this->color == new_color)
    return;

  this->color = new_color;
  this->update();

  Q_EMIT this->value_changed();
  Q_EMIT this->edit_ended();
}

QColor ColorPicker::get_color() const { return this->color; }

void ColorPicker::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QRect rect = this->rect().adjusted(0, 1, 0, -1);

  // --- Checkerboard background for alpha visualization ---
  // const int checker_size = 8;
  // QPixmap checker(checker_size * 2, checker_size * 2);
  // QPainter checker_painter(&checker);
  // checker_painter.fillRect(0, 0, checker_size * 2, checker_size * 2, Qt::white);
  // checker_painter.fillRect(0, 0, checker_size, checker_size, QColor(220, 220, 220));
  // checker_painter.fillRect(checker_size, checker_size, checker_size, checker_size,
  // QColor(220, 220, 220)); painter.fillRect(rect, QBrush(checker));

  // color overlay
  painter.setBrush(this->color);
  painter.setPen(Qt::gray);
  painter.drawRoundedRect(rect, QSX_CONFIG->global.radius, QSX_CONFIG->global.radius);

  // label
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  QFontMetrics fm(this->font());
  int          base_dx = fm.horizontalAdvance(QString("M"));
  QRectF       rect_label = rect.adjusted(base_dx, 0, -base_dx, 0);
  painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());
}

void ColorPicker::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    this->open_color_dialog();
}

void ColorPicker::open_color_dialog()
{
  QColor new_color = QColorDialog::getColor(this->color,
                                            this,
                                            "Select Color",
                                            QColorDialog::ShowAlphaChannel |
                                                QColorDialog::DontUseNativeDialog);

  if (new_color.isValid())
    this->set_color(new_color);
}

QSize ColorPicker::sizeHint() const { return QSize(QSX_CONFIG->global.width_min, 24); }

} // namespace qsx
