/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include <string>

#include <QWidget>

#define SFLOAT(v) static_cast<float>(v)
#define SINT(v) static_cast<int>(v)

namespace qsx
{

void resize_font(QWidget *widget, int relative_size_modification);

int text_width(QWidget *widget, const std::string &text);

std::string truncate_string(const std::string &input, size_t max_length);

} // namespace qsx