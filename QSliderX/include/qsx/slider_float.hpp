/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <cfloat> // FLT_MAX
#include <deque>

#include <QLineEdit>
#include <QWidget>

namespace qsx
{

class SliderFloat : public QWidget
{
  Q_OBJECT

public:
  SliderFloat() = delete;

  SliderFloat(const std::string &label_ = "",
              float              value_init_ = 0,
              float              vmin_ = -FLT_MAX,
              float              vmax_ = FLT_MAX,
              bool               add_plus_minus_buttons_ = true,
              const std::string &value_format_ = "{:.3f}",
              QWidget           *parent = nullptr);

  void         apply_text_edit_value();
  float        get_value() const;
  std::string  get_value_as_string() const;
  void         set_is_dragging(bool new_state);
  virtual bool set_value(float new_value);
  void         show_context_menu();
  QSize        sizeHint() const;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

protected:
  void randomize_value();
  void update_history();
  void update_geometry();

  std::string label;
  float       value_init;
  float       value;
  float       vmin;
  float       vmax;
  bool        add_plus_minus_buttons;
  std::string value_format;
  //
  int   base_dx;
  int   base_dy;
  int   slider_width;
  int   slider_width_min;
  QRect rect_minus;
  QRect rect_plus;
  QRect rect_bar;
  //
  bool              is_hovered = false;
  bool              is_minus_hovered = false;
  bool              is_plus_hovered = false;
  bool              is_bar_hovered = false;
  bool              is_dragging = false;
  float             value_before_dragging;
  int               pos_x_before_dragging;
  std::deque<float> history;
  std::string       style_sheet;

  QLineEdit *value_edit = nullptr;
};

} // namespace qsx