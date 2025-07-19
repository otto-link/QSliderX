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
    QColor color_border = QColor("#DFE1E2");
    QColor color_hovered = QColor("#FFFFFF");
    QColor color_selected = QColor("#4772B3");
    QColor color_bg = QColor("#2B2B2B");

    float width_border = 1.f;
    float width_hovered = 1.f;
    float radius = 4.f;

    int max_label_len = 16;
  } global;

  struct Slider
  {
    int   padding_v = 8;
    int   padding_h = 4;
    float ppu = 32.f; // sensitivity, pixels/unit
    float ppu_multiplier_fine_tuning = 10.f;
  } slider;

private:
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // static member to hold the singleton instance
  static std::shared_ptr<Config> instance;
};

} // namespace qsx