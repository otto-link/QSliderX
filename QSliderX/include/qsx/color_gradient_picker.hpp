/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QColor>
#include <QGradient>
#include <QPair>
#include <QVector>
#include <QWidget>

namespace qsx
{

struct Stop
{
  qreal  position;
  QColor color;
};

struct Preset
{
  QString       name;
  QVector<Stop> stops;
};

class ColorGradientPicker : public QWidget
{
  Q_OBJECT

public:
  explicit ColorGradientPicker(const std::string &label_ = "", QWidget *parent = nullptr);

  QLinearGradient get_gradient() const;
  QVector<Stop>   get_stops() const;
  void            set_gradient(const QLinearGradient &gradient);
  void            set_presets(const std::vector<Preset> &new_presets);
  void            set_stops(const QVector<Stop> &new_stops);

  QSize sizeHint() const override;

signals:
  void value_changed(); // always
  void edit_ended();    // only end of edit
  void gradient_changed(const QLinearGradient &gradient);

protected:
  void contextMenuEvent(QContextMenuEvent *) override;
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;

private:
  void   draw_checkerboard(QPainter &painter, const QRectF &rect) const;
  int    find_stop_at_position(const QPoint &pos) const;
  QRectF stop_rect(const Stop &stop) const;
  void   sort_stops();
  void   update_gradient();
  void   show_presets_menu();

  // --- Members ---
  std::string         label;
  QVector<Stop>       stops;
  int                 selected_stop_index = -1;
  bool                dragging = false;
  std::vector<Preset> presets;
};

} // namespace qsx