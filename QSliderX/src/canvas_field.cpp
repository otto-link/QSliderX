/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <format>

#include <QEvent>
#include <QHoverEvent>
#include <QPainter>

#include "qsx/canvas_field.hpp"
#include "qsx/config.hpp"
#include "qsx/internal/logger.hpp"
#include "qsx/internal/utils.hpp"

namespace qsx
{

CanvasField::CanvasField(const std::string &label_,
                         int                field_width,
                         int                field_height,
                         const std::string &value_format_,
                         QWidget           *parent)
    : QWidget(parent), value_format(value_format_), field(field_width, field_height),
      field_angle(field_width, field_height)
{
  QSXLOG->trace("CanvasField::CanvasField");

  this->label = truncate_string(label_,
                                static_cast<size_t>(QSX_CONFIG->global.max_label_len));

  this->setFocusPolicy(Qt::StrongFocus);
  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->setToolTip(
      "Field editor\n- left-click: add\n- right-click substract\n- mousewheel: brush "
      "radius\n- CTRL + mousewheel: brush strength\n- SHIFT + left-click: smoothing");

  this->update_geometry();
}

QColor CanvasField::colormap(float v) const
{
  v = std::clamp(v, 0.f, 1.f);

  int gray = SINT(255 * v);
  return QColor(gray, gray, gray);

  int r = static_cast<int>(std::clamp(4 * v - 1.5f, 0.f, 1.f) * 255);
  int g = static_cast<int>(std::clamp(4 * v - 0.5f, 0.f, 1.f) * 255);
  int b = static_cast<int>(std::clamp(4 * (0.75f - v), 0.f, 1.f) * 255);
  return QColor(r, g, b);
}

void CanvasField::clear()
{
  this->field.clear();
  this->update();
  Q_EMIT this->value_changed();
  Q_EMIT this->edit_ended();
}

void CanvasField::draw_at(const Qt::MouseButtons &buttons)
{
  QPoint pos = this->mapFromGlobal(QCursor::pos()) - this->rect_img.topLeft();
  this->draw_at(pos, buttons);
}

void CanvasField::draw_at(const QPoint &pos, const Qt::MouseButtons &buttons)
{
  const QPoint center = pos;
  const int    ir = this->brush_radius;
  float        sign = (buttons & Qt::LeftButton) ? 1.f : -1.f;

  // TODO angle based on brush gesture
  float angle = std::atan2(SFLOAT(pos.y() - pos_previous.y()),
                           SFLOAT(pos.x() - pos_previous.x()));
  angle = 0.5f * (angle / SFLOAT(M_PI) + 1.f);
  this->pos_previous = pos;

  // value
  for (int j = -ir; j <= ir; ++j)
    for (int i = -ir; i <= ir; ++i)
    {
      int fx = center.x() + i;
      int fy = center.y() + j;

      if (fx < 0 || fy < 0 || fx >= this->field.width || fy >= this->field.height)
        continue;

      float dist = std::sqrt(SFLOAT(i * i + j * j));
      bool  inside = (dist <= SFLOAT(ir));

      if (inside)
      {
        // reduce intensity when getting close to value saturation
        float amp = sign * this->brush_strength;

        float falloff = 1.f - (dist / SFLOAT(ir)); // TODO kernel
        falloff = std::clamp(falloff, 0.0f, 1.0f);

        field.at(fx, fy) += amp * falloff;
        field.at(fx, fy) = std::clamp(this->field.at(fx, fy), 0.0f, 1.0f);
      }
    }

  this->update();
  Q_EMIT this->value_changed();
}

std::vector<float> CanvasField::get_field_data() const { return this->field.data; }

std::vector<float> CanvasField::get_field_angle_data() const
{
  return this->field_angle.data;
}

int CanvasField::get_field_height() const { return this->field.height; }

int CanvasField::get_field_width() const { return this->field.height; }

bool CanvasField::event(QEvent *event)
{
  switch (event->type())
  {
  case QEvent::HoverEnter:
  {
    this->is_hovered = true;
    this->setCursor(Qt::CrossCursor);
    this->update();
  }
  break;

  case QEvent::HoverLeave:
  {
    this->is_hovered = false;
    this->setCursor(Qt::ArrowCursor);
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    // auto  *hover = static_cast<QHoverEvent *>(event);
    // QPoint mouse_pos = hover->position().toPoint(); // mouse position inside the widget

    this->update();
  }
  break;

  default:
    break;
  }

  return QWidget::event(event);
}

bool CanvasField::is_mouse_cursor_on_img() const
{
  QPoint mouse_pos = this->mapFromGlobal(QCursor::pos());
  return this->rect_img.contains(mouse_pos);
}

void CanvasField::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Control)
  {
    this->ctrl_pressed = true;
    this->update();
  }
  else if (event->key() == Qt::Key_Shift)
  {
    this->shift_pressed = true;
    this->update();
  }
}

void CanvasField::keyReleaseEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Control)
  {
    this->ctrl_pressed = false;
    this->update();
  }
  else if (event->key() == Qt::Key_Shift)
  {
    this->shift_pressed = false;
    this->update();
  }
}

void CanvasField::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_drawing)
    this->draw_at(event->buttons());

  QWidget::mouseMoveEvent(event);
}

void CanvasField::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
  {
    this->is_drawing = true;
    this->pos_previous = this->mapFromGlobal(QCursor::pos());
    this->draw_at(event->buttons());
  }

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void CanvasField::mouseReleaseEvent(QMouseEvent *event)
{
  this->is_drawing = false;
  Q_EMIT this->edit_ended();

  // no call to the base class event handler to avoid unwanted closing
  // of context menu for instance
}

void CanvasField::paintEvent(QPaintEvent *)
{
  const int    radius = QSX_CONFIG->global.radius;
  const QPoint mouse_pos = this->mapFromGlobal(QCursor::pos());

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // borders
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));
  painter.drawRoundedRect(this->rect(), radius, radius);

  // display data
  {
    QImage image(this->field.width, this->field.height, QImage::Format_RGB32);

    for (int j = 0; j < this->field.height; ++j)
      for (int i = 0; i < this->field.width; ++i)
      {
        float  value = std::clamp(this->field.at(i, j), 0.f, 1.f);
        QColor c = this->colormap(value);
        image.setPixel(i, j, c.rgb());
      }

    painter.drawImage(this->rect_img, image);
  }

  // brush
  if (this->is_mouse_cursor_on_img())
  {
    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(mouse_pos, this->brush_radius, this->brush_radius);

    std::string txt = "";

    if (this->ctrl_pressed)
      txt = std::format("Strength: {:.3f}", this->brush_strength);
    else if (this->shift_pressed)
      txt = "Smoothing";

    if (!txt.empty())
    {
      painter.setPen(
          QPen(QSX_CONFIG->global.color_text, QSX_CONFIG->global.width_border));
      painter.setBrush(Qt::NoBrush);
      painter.drawText(this->rect_img, Qt::AlignLeft | Qt::AlignTop, txt.c_str());
    }
  }
}

void CanvasField::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void CanvasField::set_brush_strength(float new_strength)
{
  this->brush_strength = new_strength;
}

void CanvasField::set_field_data(const std::vector<float> &new_data)
{
  // no checking and the data are eventually resized to their initial
  // size (with clipping or zero filling if the dimensions mismatched...)
  this->field.data = new_data;
  this->field.data.resize(this->field.width * this->field.height);
}

QSize CanvasField::sizeHint() const
{
  return QSize(this->canvas_width, this->canvas_height);
}

void CanvasField::update_geometry()
{
  int gap = QSX_CONFIG->global.radius;

  this->canvas_width = this->field.width + 2 * gap;
  this->canvas_height = this->field.height + 2 * gap;

  this->rect_img = QRect(QPoint(gap, gap), QSize(this->field.width, this->field.height));

  // force size
  this->setMinimumWidth(this->canvas_width);
  this->setMinimumHeight(this->canvas_height);
  this->setMaximumWidth(this->canvas_width);
  this->setMaximumHeight(this->canvas_height);
  this->setFixedSize(QSize(this->canvas_width, this->canvas_height));

  this->update();
}

void CanvasField::wheelEvent(QWheelEvent *event)
{
  if (this->is_mouse_cursor_on_img())
  {
    if (event->modifiers() & Qt::ControlModifier)
    {
      // brush strength
      float diff = event->angleDelta().y() > 0 ? 1.f : -1.f;
      diff *= QSX_CONFIG->canvas.brush_strength_tick;
      this->brush_strength = std::max(0.f, this->brush_strength + diff);
    }
    else
    {
      // scale increment with brush size
      int diff = std::max(1, SINT(SFLOAT(this->brush_radius) / 8.f));
      diff *= event->angleDelta().y() > 0 ? 1 : -1;
      this->brush_radius = std::max(1, this->brush_radius + diff);
    }

    this->update();
  }
}

} // namespace qsx
