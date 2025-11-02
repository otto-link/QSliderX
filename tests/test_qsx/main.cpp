/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

#include "qsx/canvas_field.hpp"
#include "qsx/canvas_points.hpp"
#include "qsx/color_gradient_picker.hpp"
#include "qsx/color_picker.hpp"
#include "qsx/curve_editor.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/point2d_selector.hpp"
#include "qsx/slider_float.hpp"
#include "qsx/slider_float_log.hpp"
#include "qsx/slider_int.hpp"
#include "qsx/slider_range.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  qsx::Logger::log()->info("Starting test application...");

  QWidget      window;
  QVBoxLayout *layout = new QVBoxLayout(&window);

  bool show_slider_int = false;
  bool show_slider_float = false;
  bool show_slider_float_log = false;
  bool show_slider_range = false;
  bool show_point2d_selector = true;
  bool show_curve_editor = true;
  bool show_canvas_points = false;
  bool show_canvas_field = false;
  bool show_color_picker = false;
  bool show_gradient_color_picker = false;

  if (show_canvas_field) // --- CanvasField
  {
    {
      auto *s = new qsx::CanvasField("Canvas");
      layout->addWidget(s);

      s->set_allow_angle_mode(true);

      s->connect(s,
                 &qsx::CanvasField::value_changed,
                 [s]() { qsx::Logger::log()->trace("value"); });
      s->connect(s,
                 &qsx::CanvasField::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended"); });
    }
  }

  if (show_canvas_points) // --- CanvasPoints
  {
    {
      auto *s = new qsx::CanvasPoints("Canvas");
      layout->addWidget(s);

      std::vector<float> x = {0.1f, 0.5f, 0.7f};
      std::vector<float> y = {0.5f, 0.5f, 0.6f};
      s->set_points(x, y);

      s->connect(s,
                 &qsx::CanvasPoints::value_changed,
                 [s]() { qsx::Logger::log()->trace("value"); });

      s->connect(s,
                 &qsx::CanvasPoints::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended"); });
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

      s->connect(s,
                 &qsx::CanvasPoints::value_changed,
                 [s]() { qsx::Logger::log()->trace("value"); });

      s->connect(s,
                 &qsx::CanvasPoints::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended"); });
    }
  }

  if (show_slider_int) // --- SliderInt
  {
    {
      auto *s = new qsx::SliderInt("Int", 3, 0, 10);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::value_changed,
                 [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("Int", INT_MAX, 0, INT_MAX, true, "{:} K");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("NoLim", 5, -INT_MAX, INT_MAX, false, "{} it.");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderInt("Int", 5, 0, 10, false);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderInt::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      // auto *s = new qsx::SliderInt("A very long label, actually too long", INT_MAX);
      // layout->addWidget(s);

      // s->connect(s,
      //            &qsx::SliderInt::value_changed,
      //            [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      // s->connect(s,
      //            &qsx::SliderInt::edit_ended,
      //            [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value());
      //            });
    }
  }

  if (show_slider_float) // --- SliderFloat
  {
    {
      auto *s = new qsx::SliderFloat("Float", 3, 0, 10);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::value_changed,
                 [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat(
          "Float, with a very very long long long long  long long long caption",
          1e5f,
          0,
          FLT_MAX,
          true,
          "{:.3f} K");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat("NoLim", 5, -FLT_MAX, FLT_MAX, false, "{:.1f} it.");
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloat("Float", 5, 0, 10, false);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      // auto *s = new qsx::SliderFloat("A very long label, actually too long", FLT_MAX);
      // layout->addWidget(s);

      // s->connect(s,
      //            &qsx::SliderFloat::value_changed,
      //            [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      // s->connect(s,
      //            &qsx::SliderFloat::edit_ended,
      //            [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value());
      //            });
    }
  }

  if (show_slider_float_log) // --- SliderFloatLog
  {
    {
      auto *s = new qsx::SliderFloatLog("Log Float", 3, 1e-5f, 1e5f);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::value_changed,
                 [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }

    {
      auto *s = new qsx::SliderFloatLog("Log Float", 3, 1e-5f, FLT_MAX);
      layout->addWidget(s);

      s->connect(s,
                 &qsx::SliderFloat::value_changed,
                 [s]() { qsx::Logger::log()->trace("value: {}", s->get_value()); });

      s->connect(s,
                 &qsx::SliderFloat::edit_ended,
                 [s]() { qsx::Logger::log()->trace("edit ended: {}", s->get_value()); });
    }
  }

  if (show_slider_range) // --- SliderRange
  {
    {
      auto *r = new qsx::SliderRange("Range", 2.f, 7.f, 0.f, 10.f, "{:.3f}°");
      layout->addWidget(r);

      r->connect(
          r,
          &qsx::SliderRange::value_changed,
          [r]() {
            qsx::Logger::log()->trace("value: {} {}", r->get_value(0), r->get_value(1));
          });

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   qsx::Logger::log()->trace("edit ended: {} {}",
                                             r->get_value(0),
                                             r->get_value(1));
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

      r->connect(
          r,
          &qsx::SliderRange::value_changed,
          [r]() {
            qsx::Logger::log()->trace("value: {} {}", r->get_value(0), r->get_value(1));
          });

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   qsx::Logger::log()->trace("edit ended: {} {}",
                                             r->get_value(0),
                                             r->get_value(1));
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

      r->connect(
          r,
          &qsx::SliderRange::value_changed,
          [r]() {
            qsx::Logger::log()->trace("value: {} {}", r->get_value(0), r->get_value(1));
          });

      r->connect(r,
                 &qsx::SliderRange::edit_ended,
                 [r]() {
                   qsx::Logger::log()->trace("edit ended: {} {}",
                                             r->get_value(0),
                                             r->get_value(1));
                 });
    }
  }

  if (show_point2d_selector)
  {
    auto *r = new qsx::Point2DSelector("Test", -1.f, 1.f, -0.5f, 1.f);
    layout->addWidget(r);

    r->connect(r,
               &qsx::Point2DSelector::value_changed,
               [r]()
               {
                 qsx::Logger::log()->trace("value: {} {}",
                                           r->get_value().first,
                                           r->get_value().second);
               });

    r->connect(r,
               &qsx::Point2DSelector::edit_ended,
               [r]()
               {
                 qsx::Logger::log()->trace("edit ended: {} {}",
                                           r->get_value().first,
                                           r->get_value().second);
               });
  }

  if (show_curve_editor)
  {
    auto *r = new qsx::CurveEditor("Curve");
    layout->addWidget(r);

    r->connect(r,
               &qsx::CurveEditor::value_changed,
               [r]()
               {
                 for (auto &v : r->get_values())
                   qsx::Logger::log()->trace("value: {}", v);
               });

    r->connect(r,
               &qsx::CurveEditor::edit_ended,
               [r]()
               {
                 for (auto &v : r->get_values())
                   qsx::Logger::log()->trace("-- edit ended: {}", v);
               });
  }

  if (show_color_picker)
  {
    auto *r = new qsx::ColorPicker("Color");
    layout->addWidget(r);
  }

  if (show_gradient_color_picker)
  {
    auto *r = new qsx::ColorGradientPicker("Color");
    layout->addWidget(r);

    r->set_presets({
        {"Black to White", {{0.0, QColor(0, 0, 0)}, {1.0, QColor(255, 255, 255)}}},
        {"Rainbow",
         {{0.0, QColor(255, 0, 0)},
          {0.2, QColor(255, 255, 0)},
          {0.4, QColor(0, 255, 0)},
          {0.6, QColor(0, 255, 255)},
          {0.8, QColor(0, 0, 255)},
          {1.0, QColor(255, 0, 255)}}},
        {"Fire",
         {{0.0, QColor(0, 0, 0)},
          {0.3, QColor(180, 0, 0)},
          {0.6, QColor(255, 128, 0)},
          {1.0, QColor(255, 255, 0)}}},
        {"Ocean",
         {{0.0, QColor(0, 32, 64)},
          {0.5, QColor(0, 128, 255)},
          {1.0, QColor(192, 255, 255)}}},
        {"Forest",
         {{0.0, QColor(32, 64, 0)},
          {0.5, QColor(64, 128, 64)},
          {1.0, QColor(192, 255, 192)}}},
    });

    r->connect(r,
               &qsx::ColorGradientPicker::gradient_changed,
               [&](const QLinearGradient &grad)
               { qsx::Logger::log()->trace("value changed: {}", grad.stops().size()); });
    r->connect(r,
               &qsx::ColorGradientPicker::edit_ended,
               []() { qsx::Logger::log()->trace("edit ended\n"); });
  }

  window.show();

  return app.exec();
}
