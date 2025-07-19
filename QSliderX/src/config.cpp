/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include "qsx/config.hpp"

namespace qsx
{

std::shared_ptr<Config> Config::instance = nullptr;

std::shared_ptr<Config> &Config::get_config()
{
  if (!instance)
    instance = std::make_shared<Config>();
  return instance;
}

} // namespace qsx
