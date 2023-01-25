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

#include "Macros.h"
#include "Model/CompilationTask.h"

#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QProcess> // for QProcess::ProcessError

namespace TrenchBroom
{
namespace Model
{
class CompilationProfile;
} // namespace Model

namespace View
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
  std::string interpolate(const std::string& spec);

private:
  virtual void doExecute() = 0;
  virtual void doTerminate() = 0;

  deleteCopyAndMove(CompilationTaskRunner);
};

class CompilationExportMapTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  Model::CompilationExportMap m_task;

public:
  CompilationExportMapTaskRunner(
    CompilationContext& context, Model::CompilationExportMap task);
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
  Model::CompilationCopyFiles m_task;

public:
  CompilationCopyFilesTaskRunner(
    CompilationContext& context, Model::CompilationCopyFiles task);
  ~CompilationCopyFilesTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

  deleteCopyAndMove(CompilationCopyFilesTaskRunner);
};

class CompilationDeleteFilesTaskRunner : public CompilationTaskRunner
{
  Q_OBJECT
private:
  Model::CompilationDeleteFiles m_task;

public:
  CompilationDeleteFilesTaskRunner(
    CompilationContext& context, Model::CompilationDeleteFiles task);
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
  Model::CompilationRunTool m_task;
  QProcess* m_process{nullptr};
  bool m_terminated{false};

public:
  CompilationRunToolTaskRunner(
    CompilationContext& context, Model::CompilationRunTool task);
  ~CompilationRunToolTaskRunner() override;

private:
  void doExecute() override;
  void doTerminate() override;

private:
  void startProcess();
  std::string cmd();
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

  std::unique_ptr<CompilationContext> m_context;
  TaskRunnerList m_taskRunners;
  TaskRunnerList::iterator m_currentTask;

public:
  CompilationRunner(
    std::unique_ptr<CompilationContext> context,
    const Model::CompilationProfile& profile,
    QObject* parent = nullptr);
  ~CompilationRunner() override;

private:
  static TaskRunnerList createTaskRunners(
    CompilationContext& context, const Model::CompilationProfile& profile);

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
} // namespace View
} // namespace TrenchBroom
