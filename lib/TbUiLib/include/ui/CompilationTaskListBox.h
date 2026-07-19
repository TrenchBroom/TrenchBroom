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

#include "mdl/CompilationTask.h"
#include "ui/ControlListBox.h"

#include <vector>

class QCheckBox;
class QCompleter;
class QComboBox;
class QHBoxLayout;
class QLayout;
class QLineEdit;
class QWidget;

namespace tb
{
namespace mdl
{
struct CompilationProfile;
} // namespace mdl

namespace ui
{
class MapDocument;
class MultiCompletionLineEdit;
class TitledPanel;

class CompilationTaskEditorBase : public ControlListBoxItemRenderer
{
  Q_OBJECT
protected:
  const QString m_title;
  MapDocument& m_document;
  mdl::CompilationProfile& m_profile;
  mdl::CompilationTask& m_task;
  QCheckBox* m_enabledCheckbox = nullptr;
  QHBoxLayout* m_taskLayout = nullptr;

  std::vector<QCompleter*> m_completers;

protected:
  CompilationTaskEditorBase(
    QString title,
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent);

  void setupCompleter(MultiCompletionLineEdit* lineEdit);
  void addMainLayout(QLayout* layout);

public:
  void updateItem() override;

private:
  void updateCompleter(QCompleter* completer);
};

class CompilationExportMapTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_targetEditor = nullptr;
  QLineEdit* m_stripEntityPattern = nullptr;
  QLineEdit* m_dropEntity = nullptr;
  QCheckBox* m_stripTbProperties = nullptr;

public:
  CompilationExportMapTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationExportMap& task();
private slots:
  void targetSpecChanged(const QString& text);
  void stripEntityPatternChanged(const QString& text);
  void dropEntityChanged(const QString& text);
  void stripTbPropertiesChanged(int state);
};

class CompilationCopyFilesTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_sourceEditor = nullptr;
  MultiCompletionLineEdit* m_targetEditor = nullptr;

public:
  CompilationCopyFilesTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationCopyFiles& task();
private slots:
  void sourceSpecChanged(const QString& text);
  void targetSpecChanged(const QString& text);
};

class CompilationRenameFileTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_sourceEditor = nullptr;
  MultiCompletionLineEdit* m_targetEditor = nullptr;

public:
  CompilationRenameFileTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationRenameFile& task();
private slots:
  void sourceSpecChanged(const QString& text);
  void targetSpecChanged(const QString& text);
};

class CompilationDeleteFilesTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_targetEditor = nullptr;

public:
  CompilationDeleteFilesTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationDeleteFiles& task();
private slots:
  void targetSpecChanged(const QString& text);
};

class CompilationRunToolTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_toolEditor = nullptr;
  MultiCompletionLineEdit* m_parametersEditor = nullptr;
  QCheckBox* m_treatNonZeroResultCodeAsError = nullptr;

public:
  CompilationRunToolTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationRunTool& task();
private slots:
  void browseTool();
  void toolSpecChanged(const QString& text);
  void parameterSpecChanged(const QString& text);
  void treatNonZeroResultCodeAsErrorChanged(int state);
};

class CompilationLaunchEngineTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  QComboBox* m_engineProfileEditor = nullptr;
  QCheckBox* m_treatLaunchFailureAsError = nullptr;

public:
  CompilationLaunchEngineTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  mdl::CompilationLaunchEngine& task();
private slots:
  void engineProfileChanged(int index);
  void treatLaunchFailureAsErrorChanged(int state);
};

class CompilationTaskListBox : public ControlListBox
{
  Q_OBJECT
private:
  MapDocument& m_document;
  mdl::CompilationProfile* m_profile = nullptr;

public:
  explicit CompilationTaskListBox(MapDocument& document, QWidget* parent = nullptr);

  void setProfile(mdl::CompilationProfile* profile);

public:
  void reloadTasks();
  void updateTasks();

private:
  size_t itemCount() const override;
  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
signals:
  void taskContextMenuRequested(const QPoint& globalPos, mdl::CompilationTask& task);
};

} // namespace ui
} // namespace tb
