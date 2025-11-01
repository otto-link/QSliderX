/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include <QWidget>
#include <utility>

namespace qsx
{

// single 2D point (x, y) selection within a bounding box
class Point2DSelector : public QWidget
{
  Q_OBJECT

public:
  Point2DSelector(const std::string &label_ = "",
                  float              xmin_ = 0.f,
                  float              xmax_ = 1.f,
                  float              ymin_ = 0.f,
                  float              ymax_ = 1.f,
                  QWidget           *parent = nullptr);

  QSize sizeHint() const override;

  void                    set_value(const std::pair<float, float> &v);
  std::pair<float, float> get_value() const;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  std::string             label;
  float                   xmin, xmax, ymin, ymax;
  std::pair<float, float> value; // (x, y)
  bool                    is_dragging = false;
  bool                    is_hovered = false;
  bool                    is_point_hovered = false;

  QPointF                 map_to_widget(const std::pair<float, float> &v) const;
  std::pair<float, float> map_from_widget(const QPoint &p) const;
};

} // namespace qsx
