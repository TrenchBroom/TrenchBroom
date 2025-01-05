/*
 Copyright (C) 2010 Kristian Duske

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

#include <QString>
#include <QTableView>

namespace tb::ui
{

/**
 * Hardcoded shortcuts:
 * - Ctrl+Enter emits the `addRowShortcutTriggered` signal
 * - Delete or Backspace emits the `removeRowsShortcutTriggered` signal
 */
class EntityPropertyTable : public QTableView
{
  Q_OBJECT
private:
  bool m_mousePressedOnSelectedCell = false;

public:
  explicit EntityPropertyTable(QWidget* parent = nullptr);

  static QString insertRowShortcutString();
  static QString removeRowShortcutString();
  void finishEditing(QWidget* editor);

protected:
  bool event(QEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void initViewItemOption(QStyleOptionViewItem* option) const override;
  void keyboardSearch(const QString& search) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
signals:
  void addRowShortcutTriggered();
  void removeRowsShortcutTriggered();
};

} // namespace tb::ui
