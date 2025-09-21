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
class QHBoxLayout;
class QLayout;
class QLineEdit;
class QWidget;

namespace tb::mdl
{
struct CompilationProfile;
} // namespace tb::mdl

namespace tb::ui
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

protected:
  void setupCompleter(MultiCompletionLineEdit* lineEdit);
  void addMainLayout(QLayout* layout);

protected:
  void updateItem() override;

private:
  void updateCompleter(QCompleter* completer);
};

class CompilationExportMapTaskEditor : public CompilationTaskEditorBase
{
  Q_OBJECT
private:
  MultiCompletionLineEdit* m_targetEditor = nullptr;

public:
  CompilationExportMapTaskEditor(
    MapDocument& document,
    mdl::CompilationProfile& profile,
    mdl::CompilationTask& task,
    QWidget* parent = nullptr);

private:
  void updateItem() override;
  mdl::CompilationExportMap& task();
private slots:
  void targetSpecChanged(const QString& text);
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

private:
  void updateItem() override;
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

private:
  void updateItem() override;
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

private:
  void updateItem() override;
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

private:
  void updateItem() override;
  mdl::CompilationRunTool& task();
private slots:
  void browseTool();
  void toolSpecChanged(const QString& text);
  void parameterSpecChanged(const QString& text);
  void treatNonZeroResultCodeAsErrorChanged(int state);
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

private:
  size_t itemCount() const override;
  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
signals:
  void taskContextMenuRequested(const QPoint& globalPos, mdl::CompilationTask& task);
};

} // namespace tb::ui
