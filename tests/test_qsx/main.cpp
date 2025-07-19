/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

#include "qsx/logger.hpp"
#include "qsx/slider_int.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QSXLOG->info("Starting test application...");

  QWidget window;
  // window.setMinimumSize(300, 200);
  QVBoxLayout *layout = new QVBoxLayout(&window);

  {
    auto *label = new QLabel("Sliders Int");
    layout->addWidget(label);
  }

  {
    auto *s = new qsx::SliderInt("Int", 3, 0, 10);
    layout->addWidget(s);

    s->connect(s,
               &qsx::SliderInt::value_changed,
               [s]() { QSXLOG->trace("value: {}", s->get_value()); });

    s->connect(s,
               &qsx::SliderInt::value_has_changed,
               [s]() { QSXLOG->trace("value has changed: {}", s->get_value()); });
  }

  {
    auto *s = new qsx::SliderInt("Int", 5, 0, 10, false);
    layout->addWidget(s);

    s->connect(s,
               &qsx::SliderInt::value_has_changed,
               [s]() { QSXLOG->trace("value has changed: {}", s->get_value()); });
  }

  {
    auto *s = new qsx::SliderInt("A very long label, actually too long");
    layout->addWidget(s);
  }

  window.show();

  return app.exec();
}
