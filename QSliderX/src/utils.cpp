/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <string>

namespace qsx
{

std::string truncate_string(const std::string &input, size_t max_length)
{
  const std::string ellipsis = "...";

  if (input.length() <= max_length)
    return input;

  if (max_length <= ellipsis.length())
    return input.substr(0, max_length); // Not enough room for "..."

  return input.substr(0, max_length - ellipsis.length()) + ellipsis;
}

} // namespace qsx
