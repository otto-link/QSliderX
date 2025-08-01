/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include <memory>

#include <QColor>

#define QSX_CONFIG qsx::Config::get_config()

namespace qsx
{

class Config
{
public:
  Config() = default;
  static std::shared_ptr<Config> &get_config();

  struct Global
  {
    QColor color_text = QColor("#FFFFFF");
    QColor color_border = QColor("#5B5B5B");
    QColor color_hovered = QColor("#FFFFFF");
    QColor color_selected = QColor("#4772B3");
    QColor color_faded = QColor("#5B5B5B");
    QColor color_bg = QColor("#2B2B2B");

    float width_border = 1.f;
    float width_hovered = 1.f;
    int   radius = 4;

    int    max_label_len = 16;
    size_t max_history = 8;
  } global;

  struct Canvas
  {
    int   point_radius = 7;
    int   value_arc_width = 6;
    float ppu_multiplier_fine_tuning = 10.f;
    float wheel_diff = 0.05f;
    float wheel_multiplier_fine_tuning = 10.f;
  } canvas;

  struct Slider
  {
    int   padding_v = 8;
    int   padding_h = 4;
    int   padding_middle = 8;
    float ppu = 1.f; // sensitivity, pixels/unit
    float ppu_multiplier_fine_tuning = 10.f;
    float button_ticks = 50.f;
  } slider;

  struct Range
  {
    int handle_radius = 5;
  } range;

private:
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // static member to hold the singleton instance
  static std::shared_ptr<Config> instance;
};

} // namespace qsx
