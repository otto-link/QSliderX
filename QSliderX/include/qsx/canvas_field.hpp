/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QImage>
#include <QWidget>

#include "qsx/internal/float_field.hpp"

namespace qsx
{

class CanvasField : public QWidget
{
  Q_OBJECT

public:
  CanvasField() = delete;

  CanvasField(const std::string &label_ = "",
              int                field_width = 512,
              int                field_height = 256,
              const std::string &value_format_ = "{:.2f}",
              QWidget           *parent = nullptr);

  void               clear();
  std::vector<float> get_field_data() const;
  std::vector<float> get_field_angle_data() const;
  int                get_field_height() const;
  int                get_field_width() const;
  void               set_allow_angle_mode(bool new_state);
  void               set_brush_strength(float new_strength);
  void               set_field_data(const std::vector<float> &new_data);

  QSize sizeHint() const override;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  //   void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  QColor colormap(float v) const;
  void   draw_at(const Qt::MouseButtons &buttons);
  void   draw_at(const QPoint &pos, const Qt::MouseButtons &buttons);
  bool   is_mouse_cursor_on_img() const;
  void   update_geometry();

  std::string label;
  std::string value_format;
  FloatField  field = FloatField(0, 0);
  FloatField  field_angle = FloatField(0, 0); // in [0, 1] == [-pi, pi]
  bool        allow_angle_mode = false;
  //
  int   canvas_width;
  int   canvas_height;
  QRect rect_img;
  //
  bool             is_hovered = false;
  bool             ctrl_pressed = false;
  bool             shift_pressed = false;
  bool             is_drawing = false;
  bool             angle_mode = false;
  Qt::MouseButtons drawing_buttons;
  int              brush_radius = 32;
  float            brush_strength = 0.05f;
  QPoint           pos_previous;
};

} // namespace qsx