/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

namespace qsx
{

struct Stop; // forward

class GradientPreviewWidget : public QWidget
{
  Q_OBJECT

public:
  GradientPreviewWidget(const QString       &name_,
                        const QVector<Stop> &stops_,
                        QWidget             *parent = nullptr);

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;

private:
  QString       name;
  QVector<Stop> stops;
};

} // namespace qsx