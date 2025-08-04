/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <vector>

namespace qsx
{

struct FloatField
{
  FloatField(int w, int h) : width(w), height(h), data(w * h, 0.0f) {}

  float       &at(int x, int y) { return data[y * width + x]; }
  const float &at(int x, int y) const { return data[y * width + x]; }
  void         clear(float value = 0.0f) { std::fill(data.begin(), data.end(), value); }

  int                width, height;
  std::vector<float> data;
};

} // namespace qsx