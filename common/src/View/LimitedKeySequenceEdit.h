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

#include <QKeySequenceEdit>

namespace TrenchBroom
{
namespace View
{
class LimitedKeySequenceEdit : public QKeySequenceEdit
{
  Q_OBJECT
public:
  static const size_t MaxCount = 4;

private:
  size_t m_maxCount;
  size_t m_count;

public:
  explicit LimitedKeySequenceEdit(QWidget* parent = nullptr);
  explicit LimitedKeySequenceEdit(size_t maxCount, QWidget* parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent* event) override;
private slots:
  void resetCount();
};
} // namespace View
} // namespace TrenchBroom
