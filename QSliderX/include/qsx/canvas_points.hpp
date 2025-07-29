/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QImage>
#include <QWidget>

namespace qsx
{

class CanvasPoints : public QWidget
{
  Q_OBJECT

public:
  CanvasPoints() = delete;

  CanvasPoints(const std::string &label_ = "",
               float              xmin_ = 0.f,
               float              xmax_ = 1.f,
               float              ymin_ = 0.f,
               float              ymax_ = 1.f,
               const std::string &value_format_ = "{:.2f}",
               QWidget           *parent = nullptr);

  std::vector<float> get_points_x() const;
  std::vector<float> get_points_y() const;
  std::vector<float> get_points_z() const;
  void               set_bg_image(const QImage &new_bg_image);
  void               set_connected_points(bool new_state);
  void               set_draw_z_value(bool new_state);
  std::string        get_value_as_string(float v) const;
  void               set_is_dragging(bool new_state);
  void set_points(const std::vector<float> &new_x, const std::vector<float> &new_y);
  void set_points(const std::vector<float> &new_x,
                  const std::vector<float> &new_y,
                  const std::vector<float> &new_z);
  void set_points_x(const std::vector<float> &new_x);
  void set_points_y(const std::vector<float> &new_y);
  void set_points_z(const std::vector<float> &new_z);

  QSize sizeHint() const override;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void   add_point(float x, float y);
  void   canvas_position_to_xy(QPoint pos, float &x, float &y);
  void   remove_point(int idx);
  void   update_geometry();
  QPoint xy_to_canvas_position(float x, float y);

  std::string        label;
  float              xmin;
  float              xmax;
  float              ymin;
  float              ymax;
  std::string        value_format;
  bool               connected_points = false;
  bool               draw_z_value = true;
  std::vector<float> points_x = {};
  std::vector<float> points_y = {};
  std::vector<float> points_z = {}; // value at pt in [0, 1]
  QImage             bg_image = QImage();
  //
  int   base_dx;
  int   base_dy;
  int   canvas_width;
  int   canvas_height;
  QRect rect_points;
  QRect rect_label;
  //
  bool   is_dragging = false;
  bool   is_hovered = false;
  int    hovered_point_id = -1;
  float  value_x_before_dragging;
  float  value_y_before_dragging;
  QPoint mouse_pos_before_dragging;
};

} // namespace qsx
