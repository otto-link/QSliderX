/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include <vector>

#include <QWidget>

namespace qsx
{

class VectorEditor : public QWidget
{
  Q_OBJECT

public:
  explicit VectorEditor(const std::string &label,
                        std::vector<float> values,
                        QWidget           *parent = nullptr);

  void                      set_values(const std::vector<float> &v);
  const std::vector<float> &get_values() const;

  QSize sizeHint() const override;

signals:
  void value_changed();
  void edit_ended();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  bool event(QEvent *event) override;

private:
  std::string        label;
  std::vector<float> values;

  int  active_index = -1;
  bool is_hovered = false;
  bool is_dragging = false;

  // coordinate helpers
  QRectF  value_area() const;
  QPointF index_to_screen(int i) const;
  float   screen_to_value(int y) const;
  int     screen_to_index(int x) const;
};

} // namespace qsx