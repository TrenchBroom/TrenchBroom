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

#pragma once

#include <QWidget>

class QKeySequence;
class QAbstractButton;

namespace TrenchBroom
{
namespace View
{
class LimitedKeySequenceEdit;

class KeySequenceEdit : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY
               keySequenceChanged USER true)
private:
  LimitedKeySequenceEdit* m_keySequenceEdit;
  QAbstractButton* m_clearButton;

public:
  explicit KeySequenceEdit(QWidget* parent = nullptr);
  explicit KeySequenceEdit(size_t maxCount, QWidget* parent = nullptr);

  const QKeySequence keySequence() const;
public slots:
  void setKeySequence(const QKeySequence& keySequence);
  void clear();
signals:
  void editingFinished();
  void keySequenceChanged(const QKeySequence& keySequence);
};
} // namespace View
} // namespace TrenchBroom
