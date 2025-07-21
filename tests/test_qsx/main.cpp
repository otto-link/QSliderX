/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

#include "qsx/internal/logger.hpp"
#include "qsx/slider_int.hpp"
#include "qsx/slider_range.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QSXLOG->info("Starting test application...");

  QWidget window;
  window.setStyleSheet("background-color: black;");
  QVBoxLayout *layout = new QVBoxLayout(&window);

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
    auto *s = new qsx::SliderInt("Int", INT_MAX, 0, INT_MAX, true, "{:} K");
    layout->addWidget(s);

    s->connect(s,
               &qsx::SliderInt::value_has_changed,
               [s]() { QSXLOG->trace("value has changed: {}", s->get_value()); });
  }

  {
    auto *s = new qsx::SliderInt("NoLim", 5, -INT_MAX, INT_MAX, false, "{} it.");
    layout->addWidget(s);

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
    // auto *s = new qsx::SliderInt("A very long label, actually too long", INT_MAX);
    // layout->addWidget(s);

    // s->connect(s,
    //            &qsx::SliderInt::value_changed,
    //            [s]() { QSXLOG->trace("value: {}", s->get_value()); });

    // s->connect(s,
    //            &qsx::SliderInt::value_has_changed,
    //            [s]() { QSXLOG->trace("value has changed: {}", s->get_value()); });
  }

  {
    auto *r = new qsx::SliderRange("Range", 2.f, 7.f, 0.f, 10.f, "{:.3f}°");
    layout->addWidget(r);

    r->connect(r,
               &qsx::SliderRange::value_changed,
               [r]()
               { QSXLOG->trace("value: {} {}", r->get_value(0), r->get_value(1)); });

    r->connect(
        r,
        &qsx::SliderRange::value_has_changed,
        [r]()
        { QSXLOG->trace("value has changed: {} {}", r->get_value(0), r->get_value(1)); });
  }

  {
    auto *r = new qsx::SliderRange("Range", 2.f, 7.f, 0.f, 10.f, "{:.2f}");
    layout->addWidget(r);

    auto lambda = []()
    {
      // bin center / bin sum
      std::pair<std::vector<float>, std::vector<float>> hist = {
          {-2, -1, -0.5, 0, 0.1, 0.2, 0.3, 0.5, 5, 10},
          {1, 3, 4, 10, 12, 3, 0, 4, 10, 1}};
      return hist;
    };

    r->set_histogram_fct(lambda);

    r->connect(r,
               &qsx::SliderRange::value_changed,
               [r]()
               { QSXLOG->trace("value: {} {}", r->get_value(0), r->get_value(1)); });

    r->connect(
        r,
        &qsx::SliderRange::value_has_changed,
        [r]()
        { QSXLOG->trace("value has changed: {} {}", r->get_value(0), r->get_value(1)); });
  }

  window.show();

  return app.exec();
}
