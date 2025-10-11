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

    int    max_label_len = 64;
    size_t max_history = 8;
  } global;

  struct Canvas
  {
    int    point_radius = 7;
    int    value_arc_width = 6;
    float  ppu_multiplier_fine_tuning = 10.f;
    float  wheel_diff = 0.05f;
    float  wheel_multiplier_fine_tuning = 10.f;
    float  brush_strength_tick = 0.001f;
    int    brush_avg_radius = 5;
    QColor brush_color = QColor("#47B36B");
    QColor brush_angle_mode_color = QColor("#4772B3");
    int    brush_width = 2;
  } canvas;

  struct Slider
  {
    int   padding_v = 8;
    int   padding_h = 4;
    int   padding_middle = 8;
    float ppu = 1.f;     // sensitivity, pixels/unit
    float ppu_f = 100.f; // sensitivity, float
    float ppu_multiplier_fine_tuning = 10.f;
    float button_ticks = 50.f;
  } slider;

  struct Range
  {
    int handle_radius = 5;
  } range;

  struct ColorPicker
  {
    int   width_min = 256;
    int   height_min = 64;
    float preview_width_ratio = 0.3f;
  } color_picker;

private:
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // static member to hold the singleton instance
  static std::shared_ptr<Config> instance;
};

} // namespace qsx
