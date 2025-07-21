/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <cfloat> // FLT_MAX
#include <deque>
#include <functional>
#include <utility>

#include <QLineEdit>
#include <QWidget>

namespace qsx
{

using PairVec = std::pair<std::vector<float>, std::vector<float>>;

class SliderRange : public QWidget
{
  Q_OBJECT

public:
  SliderRange() = delete;

  SliderRange(const std::string &label_ = "",
              float              value0_init_ = 0,
              float              value1_init_ = 0,
              float              vmin_ = -FLT_MAX,
              float              vmax_ = FLT_MAX,
              const std::string &value_format_ = "{}",
              QWidget           *parent = nullptr);

  float       get_value(int id) const;
  std::string get_value_as_string(int id) const;
  float       get_vmax() const;
  float       get_vmin() const;
  void        set_is_dragging(bool new_state);
  bool        set_value(int id, float new_value);

  void set_histogram_fct(std::function<PairVec()> new_histogram_fct);

  QSize sizeHint() const override;

signals:
  void value_changed();     // always
  void value_has_changed(); // only end of edit

public slots:
  void update_bins();

protected:
  bool event(QEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  void update_geometry();
  void update_value_positions();

  std::string              label;
  float                    value0_init;
  float                    value1_init;
  float                    value0;
  float                    value1;
  float                    vmin;
  float                    vmax;
  std::string              value_format;
  PairVec                  bins;
  std::function<PairVec()> histogram_fct = nullptr;
  //
  int   base_dx;
  int   base_dy;
  int   slider_width;
  int   slider_height;
  QRect rect_bar;
  QRect rect_label;
  QRect rect_handle_min;
  QRect rect_handle_max;
  QRect rect_range;
  //
  bool  is_hovered = false;
  bool  is_min_hovered = false;
  bool  is_max_hovered = false;
  bool  is_range_hovered = false;
  bool  is_dragging = false;
  int   dragged_value_id; // min or max
  float value_before_dragging;
  int   pos_x_before_dragging;
};

} // namespace qsx