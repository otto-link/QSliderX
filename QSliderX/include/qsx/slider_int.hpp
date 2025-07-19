/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <climits> // INT_MAX

#include <QWidget>

namespace qsx
{

class SliderInt : public QWidget
{
  Q_OBJECT

public:
  SliderInt() = delete;

  SliderInt(const std::string &label_ = "",
            int                value_init_ = 0,
            int                vmin_ = -INT_MAX,
            int                vmax_ = INT_MAX,
            bool               add_plus_minus_buttons_ = true,
            QWidget           *parent = nullptr);

  int   get_value() const;
  void  set_value(int new_value);
  QSize sizeHint() const;

Q_SIGNALS:
  void value_changed();     // always
  void value_has_changed(); // only end of edit

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void update_geometry();

  std::string label;
  int         value_init;
  int         value;
  int         vmin;
  int         vmax;
  bool        add_plus_minus_buttons;
  bool        is_hovered = false;
  bool        is_minus_hovered = false;
  bool        is_plus_hovered = false;
  bool        is_bar_hovered = false;
  bool        is_dragging = false;
  int         value_before_dragging;
  int         pos_x_before_dragging;
  int         base_dx;
  int         base_dy;
  int         slider_width_min;
  QRect       rect_minus;
  QRect       rect_plus;
  QRect       rect_bar;
};

} // namespace qsx