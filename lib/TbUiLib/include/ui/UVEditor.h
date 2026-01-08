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

namespace tb
{
namespace gl
{
class ContextManager;
}

namespace mdl
{
enum class UvAxis;
enum class UvDirection;
} // namespace mdl

namespace ui
{
class Drawer;
class MapDocument;
class UVView;

class UVEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  UVView* m_uvView = nullptr;
  Drawer* m_drawer = nullptr;

  QSpinBox* m_xSubDivisionEditor = nullptr;
  QSpinBox* m_ySubDivisionEditor = nullptr;

  QAbstractButton* m_resetUVButton = nullptr;
  QAbstractButton* m_resetUVToWorldButton = nullptr;
  QAbstractButton* m_flipUAxisButton = nullptr;
  QAbstractButton* m_flipVAxisButton = nullptr;
  QAbstractButton* m_rotateUVCCWButton = nullptr;
  QAbstractButton* m_rotateUVCWButton = nullptr;

  QAbstractButton* m_alignButton = nullptr;
  QAbstractButton* m_justifyUpButton = nullptr;
  QAbstractButton* m_justifyDownButton = nullptr;
  QAbstractButton* m_justifyLeftButton = nullptr;
  QAbstractButton* m_justifyRightButton = nullptr;
  QAbstractButton* m_fitHButton = nullptr;
  QAbstractButton* m_fitVButton = nullptr;
  QAbstractButton* m_autoFitButton = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit UVEditor(
    MapDocument& document, gl::ContextManager& contextManager, QWidget* parent = nullptr);

  bool cancelMouseDrag();

private:
  void updateButtons();

private:
  void createGui(gl::ContextManager& contextManager);
  QWidget* createFitter();

  void connectObservers();

  void resetUVClicked();
  void resetUVToWorldClicked();
  void flipUVHClicked();
  void flipUVVClicked();
  void rotateUVCCWClicked();
  void rotateUVCWClicked();
  void alignClicked();
  void justifyClicked(mdl::UvAxis axis, mdl::UvDirection direction);
  void fitClicked(mdl::UvAxis axis);
  void subDivisionChanged();
};

} // namespace ui
} // namespace tb
