/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <climits> // INT_MAX
#include <deque>

#include <QLineEdit>
#include <QWidget>

namespace qsx
{

class SliderInt : public QWidget
{
  Q_OBJECT

public:
  SliderInt() = delete;

  SliderInt(const std::string &label_ = "",
            int                value_init_ = 0,
            int                vmin_ = -INT_MAX,
            int                vmax_ = INT_MAX,
            bool               add_plus_minus_buttons_ = true,
            const std::string &value_format_ = "{}",
            QWidget           *parent = nullptr);

  void        apply_text_edit_value();
  int         get_value() const;
  std::string get_value_as_string() const;
  void        set_is_dragging(bool new_state);
  bool        set_value(int new_value);
  void        show_context_menu();
  QSize       sizeHint() const;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  void randomize_value();
  void update_history();
  void update_geometry();

  std::string label;
  int         value_init;
  int         value;
  int         vmin;
  int         vmax;
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
  bool            is_hovered = false;
  bool            is_minus_hovered = false;
  bool            is_plus_hovered = false;
  bool            is_bar_hovered = false;
  bool            is_dragging = false;
  bool            force_edit_ended_emit = false;
  int             value_before_dragging;
  int             pos_x_before_dragging;
  std::deque<int> history;
  std::string     style_sheet;

  QLineEdit *value_edit = nullptr;
};

} // namespace qsx