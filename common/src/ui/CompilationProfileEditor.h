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

class QAbstractButton;
class QLineEdit;
class QStackedWidget;

namespace tb
{
namespace mdl
{
struct CompilationProfile;
} // namespace mdl

namespace ui
{
class CompilationTaskListBox;
class MapDocument;
class MultiCompletionLineEdit;

/**
 * Editor UI for a single compilation profile.
 */
class CompilationProfileEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  mdl::CompilationProfile* m_profile{nullptr};
  QStackedWidget* m_stackedWidget{nullptr};
  QLineEdit* m_nameTxt{nullptr};
  MultiCompletionLineEdit* m_workDirTxt{nullptr};
  CompilationTaskListBox* m_taskList{nullptr};
  QAbstractButton* m_addTaskButton{nullptr};
  QAbstractButton* m_removeTaskButton{nullptr};
  QAbstractButton* m_moveTaskUpButton{nullptr};
  QAbstractButton* m_moveTaskDownButton{nullptr};

public:
  explicit CompilationProfileEditor(MapDocument& document, QWidget* parent = nullptr);

private:
  QWidget* createEditorPage(QWidget* parent);

private slots:
  void nameChanged(const QString& text);
  void workDirChanged(const QString& text);

  void addTask();
  void removeTask();
  void removeTask(int index);
  void duplicateTask(int index);
  void moveTaskUp();
  void moveTaskUp(int index);
  void moveTaskDown();
  void moveTaskDown(int index);

  void taskSelectionChanged();

public:
  void setProfile(mdl::CompilationProfile* profile);

private:
  void refresh();
signals:
  /**
   * Emitted when the profile name/working directory change, or tasks are
   * added/removed/reordered.
   */
  void profileChanged();
};

} // namespace ui
} // namespace tb
