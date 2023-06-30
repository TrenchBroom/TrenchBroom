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

#include "NotifierConnection.h"

#include <memory>

class QSpinBox;
class QWidget;
class QAbstractButton;

namespace TrenchBroom
{
namespace View
{
class Selection;
class GLContextManager;
class MapDocument;
class UVView;

class UVEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  UVView* m_uvView{nullptr};
  QSpinBox* m_xSubDivisionEditor{nullptr};
  QSpinBox* m_ySubDivisionEditor{nullptr};

  QAbstractButton* m_resetTextureButton{nullptr};
  QAbstractButton* m_resetTextureToWorldButton{nullptr};
  QAbstractButton* m_flipTextureHButton{nullptr};
  QAbstractButton* m_flipTextureVButton{nullptr};
  QAbstractButton* m_rotateTextureCCWButton{nullptr};
  QAbstractButton* m_rotateTextureCWButton{nullptr};

  NotifierConnection m_notifierConnection;

public:
  explicit UVEditor(
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);

  bool cancelMouseDrag();

private:
  void updateButtons();

private:
  void createGui(GLContextManager& contextManager);

  void selectionDidChange(const Selection& selection);

  void connectObservers();

  void resetTextureClicked();
  void resetTextureToWorldClicked();
  void flipTextureHClicked();
  void flipTextureVClicked();
  void rotateTextureCCWClicked();
  void rotateTextureCWClicked();
  void subDivisionChanged();
};
} // namespace View
} // namespace TrenchBroom
