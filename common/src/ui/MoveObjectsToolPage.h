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

#include <QWidget>

#include "NotifierConnection.h"

#include <memory>

class QAbstractButton;
class QLineEdit;

namespace tb::ui
{
class MapDocument;
class Selection;

class MoveObjectsToolPage : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  QLineEdit* m_offset = nullptr;
  QAbstractButton* m_button = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit MoveObjectsToolPage(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void connectObservers();

  void createGui();
  void updateGui();

  void selectionDidChange(const Selection& selection);

  void applyMove();
};

} // namespace tb::ui
