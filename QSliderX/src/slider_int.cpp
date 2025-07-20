/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <format>
#include <random>

#include <QGraphicsSceneHoverEvent>
#include <QHoverEvent>
#include <QIntValidator>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>

#include "qsx/config.hpp"
#include "qsx/logger.hpp"
#include "qsx/slider_int.hpp"
#include "qsx/utils.hpp"

namespace qsx
{

SliderInt::SliderInt(const std::string &label_,
                     int                value_init_,
                     int                vmin_,
                     int                vmax_,
                     bool               add_plus_minus_buttons_,
                     const std::string &value_format_,
                     QWidget           *parent)
    : QWidget(parent), value_init(value_init_), value(value_init_), vmin(vmin_),
      vmax(vmax_), add_plus_minus_buttons(add_plus_minus_buttons_),
      value_format(value_format_)
{
  QSXLOG->trace("SliderInt::SliderInt");

  this->label = truncate_string(label_,
                                static_cast<size_t>(QSX_CONFIG->global.max_label_len));

  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->update_geometry();

  this->connect(this,
                &SliderInt::value_has_changed,
                [this]()
                {
                  this->update_history();
                  this->update_geometry();
                });

  // style (for standard widgets
  this->style_sheet = "background-color: " +
                      QSX_CONFIG->global.color_bg.name().toStdString() +
                      "; color: " + QSX_CONFIG->global.color_text.name().toStdString() +
                      "; border: 0px;" + "selection-background-color: #ABABAB;";

  // text edit
  this->value_edit = new QLineEdit(this);
  this->value_edit->setVisible(false);
  this->value_edit->setFixedHeight(this->height() - 2);
  this->value_edit->setAlignment(Qt::AlignCenter);
  this->connect(value_edit,
                &QLineEdit::editingFinished,
                this,
                &SliderInt::apply_text_edit_value);
  this->value_edit->setStyleSheet(this->style_sheet.c_str());
}

void SliderInt::apply_text_edit_value()
{
  int new_value = this->value_edit->text().toInt();
  if (this->set_value(new_value))
    Q_EMIT this->value_has_changed();

  this->value_edit->setVisible(false);
  this->update();
}

bool SliderInt::event(QEvent *event)
{
  switch (event->type())
  {
  case QEvent::HoverEnter:
  {
    this->is_hovered = true;
    this->update();
  }
  break;

  case QEvent::HoverLeave:
  {
    this->is_hovered = false;
    this->is_minus_hovered = false;
    this->is_plus_hovered = false;
    this->is_bar_hovered = false;
    this->update();
  }
  break;

  case QEvent::HoverMove:
  {
    auto  *hover = static_cast<QHoverEvent *>(event);
    QPoint pos = hover->position().toPoint(); // mouse position inside the widget

    this->is_minus_hovered = this->rect_minus.contains(pos);
    this->is_plus_hovered = this->rect_plus.contains(pos);
    this->is_bar_hovered = this->rect_bar.contains(pos);
    this->update();
  }
  break;

  default:
    break;
  }
  return QWidget::event(event);
}

int SliderInt::get_value() const { return this->value; }

std::string SliderInt::get_value_as_string() const
{
  return std::vformat(this->value_format, std::make_format_args(this->value));
}

void SliderInt::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (this->is_bar_hovered)
  {
    this->value_edit->setText(QString::number(this->value));
    this->value_edit->setGeometry(this->rect_bar.adjusted(1, 1, -1, -1));
    this->value_edit->setVisible(true);
    this->value_edit->setFocus();
    this->value_edit->selectAll();
    this->update();
  }

  QWidget::mouseDoubleClickEvent(event);
}

void SliderInt::mouseMoveEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    // pixels per unit
    float ppu = QSX_CONFIG->slider.ppu;

    if (event->modifiers() & Qt::ControlModifier)
      ppu *= QSX_CONFIG->slider.ppu_multiplier_fine_tuning;

    int dx = event->position().toPoint().x() - this->pos_x_before_dragging;
    int dv = static_cast<int>(static_cast<float>(dx) / ppu);
    this->set_value(this->value_before_dragging + dv);
  }

  QWidget::mouseMoveEvent(event);
}

void SliderInt::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->is_bar_hovered)
    {
      this->value_before_dragging = this->value;
      this->pos_x_before_dragging = event->position().toPoint().x();
      this->set_is_dragging(true);
    }
    else if (this->is_minus_hovered)
    {
      if (this->set_value(this->get_value() - 1))
        Q_EMIT this->value_has_changed();
    }
    else if (this->is_plus_hovered)
    {
      if (this->set_value(this->get_value() + 1))
        Q_EMIT this->value_has_changed();
    }
  }
  else if (event->button() == Qt::RightButton)
  {
    this->show_context_menu();
  }

  QWidget::mousePressEvent(event);
}

void SliderInt::mouseReleaseEvent(QMouseEvent *event)
{
  if (this->is_dragging)
  {
    this->set_is_dragging(false);

    if (this->value != this->value_before_dragging)
      Q_EMIT this->value_has_changed();
  }

  QWidget::mouseReleaseEvent(event);
}

void SliderInt::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // background and border
  painter.setBrush(QBrush(QSX_CONFIG->global.color_bg));
  painter.setPen(
      this->is_hovered
          ? QPen(QSX_CONFIG->global.color_hovered, QSX_CONFIG->global.width_hovered)
          : QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

  painter.drawRoundedRect(this->rect(),
                          QSX_CONFIG->global.radius,
                          QSX_CONFIG->global.radius);

  // value bar
  if (this->vmin != -INT_MAX && this->vmax != INT_MAX && !this->value_edit->isVisible())
  {
    const int range = this->vmax - this->vmin;
    if (range > 0)
    {
      const float r = static_cast<float>(this->value - this->vmin) /
                      static_cast<float>(range);
      if (r > 0.f)
      {
        const int rcut = static_cast<int>((1.f - r) *
                                          static_cast<float>(this->rect_bar.width()));

        painter.setBrush(QSX_CONFIG->global.color_selected);
        painter.setPen(Qt::NoPen);

        if (this->add_plus_minus_buttons)
        {
          painter.drawRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1));
        }
        else
        {
          painter.drawRoundedRect(this->rect_bar.adjusted(1, 1, -rcut - 1, -1),
                                  QSX_CONFIG->global.radius,
                                  QSX_CONFIG->global.radius);
        }
      }
    }
  }

  // draw vertical separators for minus/plus buttons
  if (this->add_plus_minus_buttons)
  {
    painter.setPen(
        QPen(QSX_CONFIG->global.color_border, QSX_CONFIG->global.width_border));

    const int x_minus = this->rect_minus.right() + 1;
    const int x_plus = this->rect_plus.left() - 1;

    painter.drawLine(QPoint(x_minus, this->rect().top()),
                     QPoint(x_minus, this->rect().bottom()));
    painter.drawLine(QPoint(x_plus, this->rect().top()),
                     QPoint(x_plus, this->rect().bottom()));
  }

  // labels
  painter.setBrush(QBrush(QSX_CONFIG->global.color_text));
  painter.setPen(QPen(QSX_CONFIG->global.color_text));

  QRect rect_label = this->rect_bar.adjusted(this->base_dx, 0, -this->base_dx, 0);
  painter.drawText(rect_label, Qt::AlignLeft | Qt::AlignVCenter, this->label.c_str());

  painter.drawText(rect_label,
                   Qt::AlignRight | Qt::AlignVCenter,
                   this->get_value_as_string().c_str());

  // arrows
  QString left = this->is_minus_hovered ? "◀" : "◁";
  QString right = this->is_plus_hovered ? "▶" : "▷";

  painter.drawText(this->rect_minus, Qt::AlignCenter | Qt::AlignVCenter, left);
  painter.drawText(this->rect_plus, Qt::AlignCenter | Qt::AlignVCenter, right);
}

void SliderInt::randomize_value()
{
  std::random_device              rd;
  std::mt19937                    gen(rd());
  std::uniform_int_distribution<> dist(vmin, vmax);

  if (this->set_value(dist(gen)))
    Q_EMIT this->value_has_changed();
}

void SliderInt::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void SliderInt::set_is_dragging(bool new_state)
{
  this->is_dragging = new_state;

  if (this->is_dragging)
    this->setCursor(Qt::SizeHorCursor);
  else
    this->setCursor(Qt::ArrowCursor);
}

bool SliderInt::set_value(int new_value)
{
  new_value = std::clamp(new_value, this->vmin, this->vmax);

  if (new_value == this->value)
  {
    return false;
  }
  else
  {
    this->value = new_value;
    this->update();
    Q_EMIT this->value_changed();
  }

  return true;
}

void SliderInt::show_context_menu()
{
  QMenu menu(this);
  menu.setStyleSheet(this->style_sheet.c_str());

  // add predefined actions
  QAction *randomize_action = menu.addAction("Randomize");
  QAction *reset_action = menu.addAction("Reset");
  menu.addSeparator()->setText("History");

  // Add history actions
  for (int i = static_cast<int>(this->history.size()) - 1; i >= 0; --i)
  {
    int      v = history[static_cast<size_t>(i)];
    QString  hist_label = QString("Set to %1").arg(v);
    QAction *history_action = menu.addAction(hist_label);
    history_action->setData(v); // store value
  }

  QAction *selected = menu.exec(QCursor::pos());

  if (selected)
  {
    if (selected == randomize_action)
    {
      this->randomize_value();
    }
    else if (selected == reset_action)
    {
      if (this->set_value(this->value_init))
        Q_EMIT this->value_has_changed();
    }
    else
    {
      QVariant v = selected->data();
      if (v.isValid())
      {
        if (this->set_value(v.toInt()))
          Q_EMIT this->value_has_changed();
      }
    }
  }

  // manually update hover state
  QPoint local_pos = this->mapFromGlobal(QCursor::pos());
  bool   inside = this->rect().contains(local_pos);

  if (inside && !this->is_hovered)
  {
    this->is_hovered = true;
    this->update();
  }
  else if (!inside && this->is_hovered)
  {
    this->is_hovered = false;
    this->is_plus_hovered = false;
    this->is_minus_hovered = false;
    this->is_bar_hovered = false;
    this->update();
  }
}

QSize SliderInt::sizeHint() const { return QSize(this->slider_width, this->base_dy); }

void SliderInt::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = fm.horizontalAdvance(QString("M"));
  this->base_dy = fm.height() + QSX_CONFIG->slider.padding_v;

  // int label_width = QSX_CONFIG->global.max_label_len * this->base_dx;
  int label_width = fm.horizontalAdvance(this->label.c_str());
  this->slider_width = label_width + QSX_CONFIG->slider.padding_middle +
                       10 * fm.horizontalAdvance(QString("0")) + 6 * this->base_dx;

  this->slider_width_min = label_width + QSX_CONFIG->slider.padding_middle +
                           fm.horizontalAdvance(this->get_value_as_string().c_str()) +
                           6 * this->base_dx;

  // rectangles
  if (this->add_plus_minus_buttons)
  {
    this->rect_minus = this->rect();
    this->rect_minus.setWidth(2 * this->base_dx);

    this->rect_plus = this->rect().adjusted(this->rect().width() - 2 * this->base_dx,
                                            0,
                                            0,
                                            0);
  }
  else
  {
    this->rect_minus = QRect();
    this->rect_plus = QRect();
  }

  int gap = this->add_plus_minus_buttons ? 2 * this->base_dx : 0;
  this->rect_bar = this->rect().adjusted(gap, 0, -gap, 0);

  // size
  this->setMinimumWidth(this->slider_width_min);
  this->setMinimumHeight(this->sizeHint().height());
  this->setMaximumHeight(this->sizeHint().height());
}

void SliderInt::update_history()
{
  if (this->history.size() >= QSX_CONFIG->global.max_history)
    this->history.pop_front();
  this->history.push_back(this->value);
}

} // namespace qsx
