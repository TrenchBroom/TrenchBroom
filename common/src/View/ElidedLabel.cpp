/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ElidedLabel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QResizeEvent>

namespace TrenchBroom {
namespace View {
ElidedLabel::ElidedLabel(const QString& text, const Qt::TextElideMode elideMode, QWidget* parent)
  : QWidget(parent)
  , m_label(new QLabel(this))
  , m_elideMode(elideMode) {
  setContentsMargins(0, 0, 0, 0);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  setText(text);
}

ElidedLabel::ElidedLabel(const Qt::TextElideMode elideMode, QWidget* parent)
  : ElidedLabel("", elideMode, parent) {}

const QString& ElidedLabel::text() const {
  return m_fullText;
}

void ElidedLabel::setText(const QString& text) {
  m_fullText = text;
  updateElidedText(width());
}

void ElidedLabel::updateElidedText(const int width) {
  m_elidedText = fontMetrics().elidedText(m_fullText, m_elideMode, width);
  m_label->setText(m_elidedText);
  if (m_elidedText.length() < m_fullText.length()) {
    m_label->setToolTip(m_fullText);
  } else {
    m_label->setToolTip("");
  }
}

QSize ElidedLabel::minimumSizeHint() const {
  return QSize(-1, m_label->sizeHint().height());
}

void ElidedLabel::resizeEvent(QResizeEvent* event) {
  updateElidedText(event->size().width());
  m_label->setGeometry(contentsRect());
}
} // namespace View
} // namespace TrenchBroom
