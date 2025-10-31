/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include "qsx/slider_float.hpp"

namespace qsx
{

class SliderFloatLog : public SliderFloat
{
  Q_OBJECT

public:
  SliderFloatLog(const std::string &label_,
                 float              value_init_,
                 float              vmin_,
                 float              vmax_,
                 bool               add_plus_minus_buttons_ = true,
                 const std::string &value_format_ = "{:.3e}",
                 QWidget           *parent = nullptr);

  bool set_value(float new_value) override;

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  float to_log(float v) const;    // map linear value → log space
  float from_log(float lv) const; // map log space → linear value

  float log_min;
  float log_max;
};

} // namespace qsx
