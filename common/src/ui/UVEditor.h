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

class QSpinBox;
class QWidget;
class QAbstractButton;

namespace tb::mdl
{
struct SelectionChange;
}

namespace tb::ui
{
class GLContextManager;
class MapDocument;
class UVView;

class UVEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  UVView* m_uvView = nullptr;
  QSpinBox* m_xSubDivisionEditor = nullptr;
  QSpinBox* m_ySubDivisionEditor = nullptr;

  QAbstractButton* m_resetUVButton = nullptr;
  QAbstractButton* m_resetUVToWorldButton = nullptr;
  QAbstractButton* m_flipUAxisButton = nullptr;
  QAbstractButton* m_flipVAxisButton = nullptr;
  QAbstractButton* m_rotateUVCCWButton = nullptr;
  QAbstractButton* m_rotateUVCWButton = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit UVEditor(
    MapDocument& document, GLContextManager& contextManager, QWidget* parent = nullptr);

  bool cancelMouseDrag();

private:
  void updateButtons();

private:
  void createGui(GLContextManager& contextManager);

  void selectionDidChange(const mdl::SelectionChange& selectionChange);

  void connectObservers();

  void resetUVClicked();
  void resetUVToWorldClicked();
  void flipUVHClicked();
  void flipUVVClicked();
  void rotateUVCCWClicked();
  void rotateUVCWClicked();
  void subDivisionChanged();
};

} // namespace tb::ui
