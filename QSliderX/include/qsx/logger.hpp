/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <memory>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define QSXLOG qsx::Logger::log()

namespace qsx
{

class Logger
{
public:
  static std::shared_ptr<spdlog::logger> &log();

private:
  Logger() = default;
  Logger(const Logger &) = delete;

  Logger &operator=(const Logger &) = delete;

  static std::shared_ptr<spdlog::logger> instance;
};

} // namespace qsx
