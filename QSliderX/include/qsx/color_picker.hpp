/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QColor>
#include <QWidget>

namespace qsx
{

class ColorPicker : public QWidget
{
  Q_OBJECT

public:
  explicit ColorPicker(const std::string &label_, QWidget *parent = nullptr);

  void   set_color(const QColor &new_color);
  QColor get_color() const;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  void  paintEvent(QPaintEvent *event) override;
  void  mousePressEvent(QMouseEvent *event) override;
  QSize sizeHint() const override;

private:
  void open_color_dialog();

  // --- Members ---
  QColor      color;
  std::string label;
};

} // namespace qsx
