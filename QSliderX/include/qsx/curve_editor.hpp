/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once

#include <utility>
#include <vector>

#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QWidget>

namespace qsx
{

class CurveEditor : public QWidget
{
  Q_OBJECT

public:
  explicit CurveEditor(const std::string &label_ = "",
                       int                sample_count_ = 8,
                       QWidget           *parent = nullptr);

  void               clear_points();
  bool               get_smooth_interpolation() const;
  std::vector<float> get_values() const;
  void               set_values(const std::vector<float> &new_values);
  void               set_sample_count(int new_sample_count);
  void               set_smooth_interpolation(bool new_state);

  QSize sizeHint() const override;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void    draw_background(QPainter &painter);
  void    draw_curve(QPainter &painter);
  void    draw_points(QPainter &painter);
  int     find_nearest_point(const QPoint &pos) const;
  QPointF point_to_screen(const QPointF &p) const;
  float   interpolate(float t) const;
  float   interpolate_catmull_rom(float t) const;
  float   interpolate_linear(float t) const;
  QPointF screen_to_point(const QPoint &p) const;
  void    update_values();

  std::string          label;
  std::vector<QPointF> control_points;
  std::vector<float>   values;
  int                  sample_count;
  bool                 smooth_interpolation = true;
  int                  active_point = -1;
  bool                 is_dragging = false;
  bool                 is_hovered = false;
};

} // namespace qsx
