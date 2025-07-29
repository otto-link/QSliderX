/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

#include "qsx/canvas_points.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/slider_float.hpp"
#include "qsx/slider_int.hpp"
#include "qsx/slider_range.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QSXLOG->info("Starting test application...");

  QWidget window;
  window.setStyleSheet("background-color: black;");
  QVBoxLayout *layout = new QVBoxLayout(&window);

  bool show_slider_int = false;
  bool show_slider_float = false;
  bool show_slider_range = false;
  bool show_canvas_points = true;

  if (show_canvas_points) // --- CanvasPoints
  {
    {
      auto *s = new qsx::CanvasPoints("Canvas");
      layout->addWidget(s);

      std::vector<float> x = {0.1f, 0.5f, 0.7f};
      std::vector<float> y = {0.5f, 0.5f, 0.6f};
      s->set_points(x, y);

      s->connect(s, &qsx::CanvasPoints::value_changed, [s]() { QSXLOG->trace("value"); });

      s->connect(s,
                 &qsx::CanvasPoints::edit_ended,
                 [s]() { QSXLOG->trace("edit ended"); });
    }

    {
      auto *s = new qsx::CanvasPoints("Canvas");
      layout->addWidget(s);

      std::vector<float> x = {0.1f, 0.5f, 0.7f};
      std::vector<float> y = {0.5f, 0.5f, 0.6f};
      s->set_points(x, y);
      s->set_connected_points(true);
      s->set_draw_z_value(false);
      s->set_bg_image(QImage("bg.png"));

      s->connect(s, &qsx::CanvasPoints::value_changed, [s]() { QSXLOG->trace("value"); });

      s->connect(s,
                 &qsx::CanvasPoints::edit_ended,
                 [s]() { QSXLOG->trace("edit ended"); });
    }
  }

  if (show_slider_int) // --- SliderInt
  {
    {
      auto *s = new qsx::SliderInt("Int", 3, 0, 10);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::value_changed,
                 [s]() { QSXLOG->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("Int", INT_MAX, 0, INT_MAX, true, "{:} K");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("NoLim", 5, -INT_MAX, INT_MAX, false, "{} it.");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("Int", 5, 0, 10, false);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      // auto *s = new qsx::SliderInt("A very long label, actually too long", INT_MAX);
      // layout->addWidget(s);

      // s->connect(s,
      //            &qsx::SliderInt::value_changed,
      //            [s]() { QSXLOG->trace("value: {}", s->get_value()); });

      // s->connect(s,
      //            &qsx::SliderInt::edit_ended,
      //            [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }
  }

  if (show_slider_float) // --- SliderFloat
  {
    {
      auto *s = new qsx::SliderFloat("Float", 3, 0, 10);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::value_changed,
                 [s]() { QSXLOG->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat("Float", 1e5f, 0, FLT_MAX, true, "{:.3f} K");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat("NoLim", 5, -FLT_MAX, FLT_MAX, false, "{:.1f} it.");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat("Float", 5, 0, 10, false);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }

    {
      // auto *s = new qsx::SliderFloat("A very long label, actually too long", FLT_MAX);
      // layout->addWidget(s);

      // s->connect(s,
      //            &qsx::SliderFloat::value_changed,
      //            [s]() { QSXLOG->trace("value: {}", s->get_value()); });

      // s->connect(s,
      //            &qsx::SliderFloat::edit_ended,
      //            [s]() { QSXLOG->trace("edit ended: {}", s->get_value()); });
    }
  }

  if (show_slider_range) // --- SliderRange
  {
    {
      auto *r = new qsx::SliderRange("Range", 2.f, 7.f, 0.f, 10.f, "{:.3f}°");
      layout->addWidget(r);

      r->connect(r,
                 &qsx::SliderRange::value_changed,
                 [r]()
                 { QSXLOG->trace("value: {} {}", r->get_value(0), r->get_value(1)); });

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   QSXLOG->trace("edit ended: {} {}", r->get_value(0), r->get_value(1));
                 });
    }

    {
      auto *r = new qsx::SliderRange("W/hist", 2.f, 7.f, 0.f, 10.f, "{:.2f}");
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

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   QSXLOG->trace("edit ended: {} {}", r->get_value(0), r->get_value(1));
                 });
    }

    {
      auto *r = new qsx::SliderRange("Auto-Range", -5.f, 7.f, 0.f, 10.f, "{:.2f}");
      layout->addWidget(r);

      auto lambda = []()
      {
        // bin center / bin sum
        std::pair<std::vector<float>, std::vector<float>> hist = {
            {-2, -1, -0.5, 0, 0.1, 0.2, 0.3, 0.5, 5, 12},
            {1, 3, 4, 10, 12, 3, 0, 4, 10, 1}};
        return hist;
      };

      r->set_histogram_fct(lambda);
      r->set_autorange(true);

      r->connect(r,
                 &qsx::SliderRange::value_changed,
                 [r]()
                 { QSXLOG->trace("value: {} {}", r->get_value(0), r->get_value(1)); });

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   QSXLOG->trace("edit ended: {} {}", r->get_value(0), r->get_value(1));
                 });
    }
  }

  window.show();

  return app.exec();
}
