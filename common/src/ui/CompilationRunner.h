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

#include <QObject>
#include <QProcess>

#include "Macros.h"
#include "Result.h"
#include "mdl/CompilationTask.h"
#include "ui/CompilationContext.h"

#include <memory>
#include <string>
#include <vector>

namespace tb::mdl
{
struct CompilationProfile;
} // namespace tb::mdl

namespace tb::ui
{
class CompilationContext;

class CompilationTaskRunner : public QObject
{
  Q_OBJECT
protected:
  CompilationContext& m_context;

protected:
  explicit CompilationTaskRunner(CompilationContext& context);

public:
  ~CompilationTaskRunner() override;

  void execute();
  void terminate();
signals:
  void start();
  void error();
  void end();

protected:
  Result<std::string> interpolate(const std::string& spec) const;

private:
  virtual void doExecute() = 0;
  virtual void doTerminate() = 0;

  deleteCopyAndMove(CompilationTaskRunner);
};

class CompilationExportMapTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  mdl::CompilationExportMap m_task;

public:
  CompilationExportMapTaskRunner(
    CompilationContext& context, mdl::CompilationExportMap task);
  ~CompilationExportMapTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

  deleteCopyAndMove(CompilationExportMapTaskRunner);
};

class CompilationCopyFilesTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  mdl::CompilationCopyFiles m_task;

public:
  CompilationCopyFilesTaskRunner(
    CompilationContext& context, mdl::CompilationCopyFiles task);
  ~CompilationCopyFilesTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

  deleteCopyAndMove(CompilationCopyFilesTaskRunner);
};

class CompilationRenameFileTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  mdl::CompilationRenameFile m_task;

public:
  CompilationRenameFileTaskRunner(
    CompilationContext& context, mdl::CompilationRenameFile task);
  ~CompilationRenameFileTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

  deleteCopyAndMove(CompilationRenameFileTaskRunner);
};

class CompilationDeleteFilesTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  mdl::CompilationDeleteFiles m_task;

public:
  CompilationDeleteFilesTaskRunner(
    CompilationContext& context, mdl::CompilationDeleteFiles task);
  ~CompilationDeleteFilesTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

  deleteCopyAndMove(CompilationDeleteFilesTaskRunner);
};

class CompilationRunToolTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  mdl::CompilationRunTool m_task;
  QProcess* m_process{nullptr};
  bool m_terminated{false};

public:
  CompilationRunToolTaskRunner(CompilationContext& context, mdl::CompilationRunTool task);
  ~CompilationRunToolTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

private:
  void startProcess();
  Result<std::string> program() const;
  Result<std::vector<std::string>> parameters() const;
private slots:
  void processErrorOccurred(QProcess::ProcessError processError);
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processReadyReadStandardError();
  void processReadyReadStandardOutput();

  deleteCopyAndMove(CompilationRunToolTaskRunner);
};

class CompilationRunner : public QObject
{
  Q_OBJECT
private:
  using TaskRunnerList = std::vector<std::unique_ptr<CompilationTaskRunner>>;

  CompilationContext m_context;
  TaskRunnerList m_taskRunners;
  TaskRunnerList::iterator m_currentTask;

public:
  CompilationRunner(
    CompilationContext context,
    const mdl::CompilationProfile& profile,
    QObject* parent = nullptr);
  ~CompilationRunner() override;

private:
  static TaskRunnerList createTaskRunners(
    CompilationContext& context, const mdl::CompilationProfile& profile);

public:
  void execute();
  void terminate();
  bool running() const;

private:
  void bindEvents(CompilationTaskRunner& runner) const;
  void unbindEvents(CompilationTaskRunner& runner) const;
private slots:
  void taskError();
  void taskEnd();
signals:
  void compilationStarted();
  void compilationEnded();

  deleteCopyAndMove(CompilationRunner);
};

} // namespace tb::ui
