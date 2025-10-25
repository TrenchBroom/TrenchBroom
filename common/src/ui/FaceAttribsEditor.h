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

class QAbstractButton;
class QLabel;
class QLineEdit;
class QGridLayout;

namespace tb
{
namespace gl
{
class ContextManager;
}

namespace ui
{
class FlagsPopupEditor;
class MapDocument;
class SignalDelayer;
class SpinControl;
class UVEditor;

class FaceAttribsEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  UVEditor* m_uvEditor = nullptr;
  QLabel* m_materialName = nullptr;
  QLabel* m_textureSize = nullptr;
  SpinControl* m_xOffsetEditor = nullptr;
  SpinControl* m_yOffsetEditor = nullptr;
  SpinControl* m_xScaleEditor = nullptr;
  SpinControl* m_yScaleEditor = nullptr;
  SpinControl* m_rotationEditor = nullptr;
  QLabel* m_surfaceValueLabel = nullptr;
  QWidget* m_surfaceValueEditorLayout = nullptr;
  SpinControl* m_surfaceValueEditor = nullptr;
  QAbstractButton* m_surfaceValueUnsetButton = nullptr;

  QLabel* m_surfaceFlagsLabel = nullptr;
  QWidget* m_surfaceFlagsEditorLayout = nullptr;
  FlagsPopupEditor* m_surfaceFlagsEditor = nullptr;
  QAbstractButton* m_surfaceFlagsUnsetButton = nullptr;
  QLabel* m_contentFlagsLabel = nullptr;
  QWidget* m_contentFlagsEditorLayout = nullptr;
  FlagsPopupEditor* m_contentFlagsEditor = nullptr;
  QAbstractButton* m_contentFlagsUnsetButton = nullptr;

  // SiN stuff
  QLabel* m_surfaceSiNNonlitValueLabel = nullptr;
  QWidget* m_surfaceSiNNonlitValueEditorLayout = nullptr;
  SpinControl* m_surfaceSiNNonlitValueEditor = nullptr;
  QAbstractButton* m_surfaceSiNNonlitValueUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNTransAngleLabel = nullptr;
  QWidget* m_surfaceSiNTransAngleEditorLayout = nullptr;
  SpinControl* m_surfaceSiNTransAngleEditor = nullptr;
  QAbstractButton* m_surfaceSiNTransAngleUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNTransMagLabel = nullptr;
  QWidget* m_surfaceSiNTransMagEditorLayout = nullptr;
  SpinControl* m_surfaceSiNTransMagEditor = nullptr;
  QAbstractButton* m_surfaceSiNTransMagUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNTranslucenceLabel = nullptr;
  QWidget* m_surfaceSiNTranslucenceEditorLayout = nullptr;
  SpinControl* m_surfaceSiNTranslucenceEditor = nullptr;
  QAbstractButton* m_surfaceSiNTranslucenceUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNRestitutionLabel = nullptr;
  QWidget* m_surfaceSiNRestitutionEditorLayout = nullptr;
  SpinControl* m_surfaceSiNRestitutionEditor = nullptr;
  QAbstractButton* m_surfaceSiNRestitutionUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNFrictionLabel = nullptr;
  QWidget* m_surfaceSiNFrictionEditorLayout = nullptr;
  SpinControl* m_surfaceSiNFrictionEditor = nullptr;
  QAbstractButton* m_surfaceSiNFrictionUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNAnimTimeLabel = nullptr;
  QWidget* m_surfaceSiNAnimTimeEditorLayout = nullptr;
  SpinControl* m_surfaceSiNAnimTimeEditor = nullptr;
  QAbstractButton* m_surfaceSiNAnimTimeUnsetButton = nullptr;

  QLabel* m_surfaceSiNDirectStyleLabel = nullptr;
  QWidget* m_surfaceSiNDirectStyleEditorLayout = nullptr;
  QLineEdit* m_surfaceSiNDirectStyleEditor = nullptr;
  QAbstractButton* m_surfaceSiNDirectStyleUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNDirectLabel = nullptr;
  QWidget* m_surfaceSiNDirectEditorLayout = nullptr;
  SpinControl* m_surfaceSiNDirectEditor = nullptr;
  QAbstractButton* m_surfaceSiNDirectUnsetButton = nullptr;
  
  QLabel* m_surfaceSiNDirectAngleLabel = nullptr;
  QWidget* m_surfaceSiNDirectAngleEditorLayout = nullptr;
  SpinControl* m_surfaceSiNDirectAngleEditor = nullptr;
  QAbstractButton* m_surfaceSiNDirectAngleUnsetButton = nullptr;


  QLabel* m_colorLabel = nullptr;
  QWidget* m_colorEditorLayout = nullptr;
  QLineEdit* m_colorEditor = nullptr;
  QAbstractButton* m_colorUnsetButton = nullptr;

  SignalDelayer* m_updateControlsSignalDelayer = nullptr;

  NotifierConnection m_notifierConnection;

public:
  FaceAttribsEditor(
    MapDocument& document, gl::ContextManager& contextManager, QWidget* parent = nullptr);

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

  // SiN stuff
  void sinNonlitValueChanged(double value);
  void sinNonlitValueUnset();
  void sinTransAngleChanged(double value);
  void sinTransAngleUnset();
  void sinTransMagChanged(double value);
  void sinTransMagUnset();
  void sinTranslucenceChanged(double value);
  void sinTranslucenceUnset();
  void sinRestitutionChanged(double value);
  void sinRestitutionUnset();
  void sinFrictionChanged(double value);
  void sinFrictionUnset();
  void sinAnimTimeChanged(double value);
  void sinAnimTimeUnset();
  void sinDirectStyleValueChanged(const QString& text);
  void sinDirectStyleValueUnset();
  void sinDirectChanged(double value);
  void sinDirectUnset();
  void sinDirectAngleChanged(double value);
  void sinDirectAngleUnset();

  void updateIncrements();

private:
  void createGui(gl::ContextManager& contextManager);
  void bindEvents();

  void connectObservers();

  void documentDidChange();

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

  // SiN stuff
  bool hasSiNAttributes() const;
  void showSiNAttribEditor();
  void hideSiNAttribEditor();

  std::tuple<QList<int>, QStringList, QStringList> getSurfaceFlags() const;
  std::tuple<QList<int>, QStringList, QStringList> getContentFlags() const;
};

} // namespace ui
} // namespace tb
