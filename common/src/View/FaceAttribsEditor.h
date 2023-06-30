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
#include <vector>

class QAbstractButton;
class QLabel;
class QLineEdit;
class QGridLayout;

namespace TrenchBroom
{
namespace Model
{
class BrushFaceHandle;
class Node;
} // namespace Model

namespace View
{
class FlagsPopupEditor;
class GLContextManager;
class MapDocument;
class Selection;
class SignalDelayer;
class SpinControl;
class UVEditor;

class FaceAttribsEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  UVEditor* m_uvEditor{nullptr};
  QLabel* m_textureName{nullptr};
  QLabel* m_textureSize{nullptr};
  SpinControl* m_xOffsetEditor{nullptr};
  SpinControl* m_yOffsetEditor{nullptr};
  SpinControl* m_xScaleEditor{nullptr};
  SpinControl* m_yScaleEditor{nullptr};
  SpinControl* m_rotationEditor{nullptr};
  QLabel* m_surfaceValueLabel{nullptr};
  QWidget* m_surfaceValueEditorLayout{nullptr};
  SpinControl* m_surfaceValueEditor{nullptr};
  QAbstractButton* m_surfaceValueUnsetButton{nullptr};

  QLabel* m_surfaceFlagsLabel{nullptr};
  QWidget* m_surfaceFlagsEditorLayout{nullptr};
  FlagsPopupEditor* m_surfaceFlagsEditor{nullptr};
  QAbstractButton* m_surfaceFlagsUnsetButton{nullptr};
  QLabel* m_contentFlagsLabel{nullptr};
  QWidget* m_contentFlagsEditorLayout{nullptr};
  FlagsPopupEditor* m_contentFlagsEditor{nullptr};
  QAbstractButton* m_contentFlagsUnsetButton{nullptr};

  QLabel* m_colorLabel{nullptr};
  QWidget* m_colorEditorLayout{nullptr};
  QLineEdit* m_colorEditor{nullptr};
  QAbstractButton* m_colorUnsetButton{nullptr};

  SignalDelayer* m_updateControlsSignalDelayer{nullptr};

  NotifierConnection m_notifierConnection;

public:
  FaceAttribsEditor(
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);

  bool cancelMouseDrag();

private:
  void xOffsetChanged(double value);
  void yOffsetChanged(double value);
  void rotationChanged(double value);
  void xScaleChanged(double value);
  void yScaleChanged(double value);
  void surfaceFlagChanged(size_t index, int value, int setFlag, int mixedFlag);
  void contentFlagChanged(size_t index, int value, int setFlag, int mixedFlag);
  void surfaceValueChanged(double value);
  void colorValueChanged(const QString& text);
  void surfaceFlagsUnset();
  void contentFlagsUnset();
  void surfaceValueUnset();
  void colorValueUnset();
  void updateIncrements();

private:
  void createGui(GLContextManager& contextManager);
  void bindEvents();

  void connectObservers();

  void documentWasNewed(MapDocument* document);
  void documentWasLoaded(MapDocument* document);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);
  void selectionDidChange(const Selection& selection);
  void textureCollectionsDidChange();

  void updateControls();
  void updateControlsDelayed();

  bool hasSurfaceFlags() const;
  bool hasContentFlags() const;
  void showSurfaceFlagsEditor();
  void showContentFlagsEditor();
  void hideSurfaceFlagsEditor();
  void hideContentFlagsEditor();

  bool hasColorAttribs() const;
  void showColorAttribEditor();
  void hideColorAttribEditor();

  std::tuple<QList<int>, QStringList, QStringList> getSurfaceFlags() const;
  std::tuple<QList<int>, QStringList, QStringList> getContentFlags() const;
};
} // namespace View
} // namespace TrenchBroom
