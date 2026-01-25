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

#include <tuple>

class QAbstractButton;
class QLabel;
class QLineEdit;
class QGridLayout;

namespace tb
{
namespace mdl
{
enum class UvAxis;
enum class UvSign;
} // namespace mdl

namespace ui
{
class AppController;
class FlagsPopupEditor;
class MapDocument;
class SignalDelayer;
class SpinControl;
class UVEditor;

class FaceAttribsEditor : public QWidget
{
  Q_OBJECT

private:
  enum class JustifyDirection
  {
    Left,
    Right,
    Up,
    Down,
  };

  enum class FitDirection
  {
    Horizontal,
    Vertical,
  };
  MapDocument& m_document;

  UVEditor* m_uvEditor = nullptr;

  QAbstractButton* m_alignButton = nullptr;
  QAbstractButton* m_justifyUpButton = nullptr;
  QAbstractButton* m_justifyDownButton = nullptr;
  QAbstractButton* m_justifyLeftButton = nullptr;
  QAbstractButton* m_justifyRightButton = nullptr;
  QAbstractButton* m_fitHButton = nullptr;
  QAbstractButton* m_fitVButton = nullptr;
  QAbstractButton* m_autoFitButton = nullptr;

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

  QLabel* m_colorLabel = nullptr;
  QWidget* m_colorEditorLayout = nullptr;
  QLineEdit* m_colorEditor = nullptr;
  QAbstractButton* m_colorUnsetButton = nullptr;

  SignalDelayer* m_updateControlsSignalDelayer = nullptr;

  NotifierConnection m_notifierConnection;

public:
  FaceAttribsEditor(
    AppController& appController, MapDocument& document, QWidget* parent = nullptr);

  bool cancelMouseDrag();

private:
  std::tuple<mdl::UvAxis, mdl::UvSign> convertJustifyDirection(
    JustifyDirection justifyDirection) const;
  std::tuple<mdl::UvAxis, mdl::UvSign> convertFitDirection(
    FitDirection fitDirection) const;

  void alignClicked();
  void justifyClicked(JustifyDirection justifyDirection);
  void fitClicked(FitDirection fitDirection);

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
  void createGui(AppController& appController);
  QWidget* createButtonsWidget();
  QWidget* createAttribsWidget();

  void bindEvents();

  void connectObservers();

  void refresh();

  void updateControls();
  void updateControlsDelayed();

  bool hasSurfaceFlags() const;
  bool hasContentFlags() const;
  void setSurfaceFlagsEditorVisible(bool visible);
  void setContentFlagsEditorVisible(bool visible);

  bool hasColorAttribs() const;
  void setColorAttribEditorVisible(bool visible);

  std::tuple<QList<int>, QStringList, QStringList> getSurfaceFlags() const;
  std::tuple<QList<int>, QStringList, QStringList> getContentFlags() const;
};

} // namespace ui
} // namespace tb
