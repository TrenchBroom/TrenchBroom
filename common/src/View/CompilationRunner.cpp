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

#include "CompilationRunner.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/ExportOptions.h"
#include "IO/Path.h"
#include "IO/PathMatcher.h"
#include "IO/PathQt.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "View/CompilationContext.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"

#include <kdl/overload.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <string>

#include <QDir>
#include <QMetaEnum>
#include <QProcess>
#include <QtGlobal>

namespace TrenchBroom
{
namespace View
{
CompilationTaskRunner::CompilationTaskRunner(CompilationContext& context)
  : m_context{context}
{
}

CompilationTaskRunner::~CompilationTaskRunner() = default;

void CompilationTaskRunner::execute()
{
  doExecute();
}

void CompilationTaskRunner::terminate()
{
  doTerminate();
}

std::string CompilationTaskRunner::interpolate(const std::string& spec)
{
  try
  {
    return m_context.interpolate(spec);
  }
  catch (const Exception& e)
  {
    m_context << "#### Could not interpolate expression '" << QString::fromStdString(spec)
              << "': " << e.what() << "\n";
    throw;
  }
}

CompilationExportMapTaskRunner::CompilationExportMapTaskRunner(
  CompilationContext& context, Model::CompilationExportMap task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationExportMapTaskRunner::~CompilationExportMapTaskRunner() = default;

void CompilationExportMapTaskRunner::doExecute()
{
  emit start();

  try
  {
    const auto targetPath = IO::Path{interpolate(m_task.targetSpec)};
    try
    {
      m_context << "#### Exporting map file '" << IO::pathAsQString(targetPath) << "'\n";

      if (!m_context.test())
      {
        IO::Disk::ensureDirectoryExists(targetPath.deleteLastComponent());

        const auto options = IO::MapExportOptions{targetPath};
        const auto document = m_context.document();
        document->exportDocumentAs(options);
      }
      emit end();
    }
    catch (const Exception& e)
    {
      m_context << "#### Could not export map file '" << IO::pathAsQString(targetPath)
                << "': " << e.what() << "\n";
      throw;
    }
  }
  catch (const Exception&)
  {
    emit error();
  }
}

void CompilationExportMapTaskRunner::doTerminate() {}

CompilationCopyFilesTaskRunner::CompilationCopyFilesTaskRunner(
  CompilationContext& context, Model::CompilationCopyFiles task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationCopyFilesTaskRunner::~CompilationCopyFilesTaskRunner() = default;

void CompilationCopyFilesTaskRunner::doExecute()
{
  emit start();

  try
  {
    const auto sourcePath = IO::Path{interpolate(m_task.sourceSpec)};
    const auto targetPath = IO::Path{interpolate(m_task.targetSpec)};

    const auto sourceDirPath = sourcePath.deleteLastComponent();
    const auto sourcePathMatcher =
      IO::makeFilenamePathMatcher(sourcePath.lastComponent().string());

    try
    {
      const auto sourcePaths = IO::Disk::find(sourceDirPath, sourcePathMatcher);
      const auto sourceStrs = kdl::vec_transform(
        sourcePaths, [](const auto& path) { return "'" + path.string() + "'"; });
      const auto sourceListQStr = QString::fromStdString(kdl::str_join(sourceStrs, ", "));
      m_context << "#### Copying to '" << IO::pathAsQString(targetPath)
                << "/': " << sourceListQStr << "\n";
      if (!m_context.test())
      {
        IO::Disk::ensureDirectoryExists(targetPath);
        IO::Disk::copyFiles(
          sourceDirPath, sourcePathMatcher, targetPath, true(overwrite));
      }
      emit end();
    }
    catch (const Exception& e)
    {
      m_context << "#### Could not copy '" << IO::pathAsQString(sourcePath) << "' to '"
                << IO::pathAsQString(targetPath) << "': " << e.what() << "\n";
      throw;
    }
  }
  catch (const Exception&)
  {
    emit error();
  }
}

void CompilationCopyFilesTaskRunner::doTerminate() {}

CompilationRenameFileTaskRunner::CompilationRenameFileTaskRunner(
  CompilationContext& context, Model::CompilationRenameFile task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationRenameFileTaskRunner::~CompilationRenameFileTaskRunner() = default;

void CompilationRenameFileTaskRunner::doExecute()
{
  emit start();

  try
  {
    const auto sourcePath = IO::Path{interpolate(m_task.sourceSpec)};
    const auto targetPath = IO::Path{interpolate(m_task.targetSpec)};

    try
    {
      m_context << "#### Renaming '" << IO::pathAsQString(sourcePath) << "' to '"
                << IO::pathAsQString(targetPath) << "'\n";
      if (!m_context.test())
      {
        IO::Disk::ensureDirectoryExists(targetPath.deleteLastComponent());
        IO::Disk::moveFile(sourcePath, targetPath, true);
      }
      emit end();
    }
    catch (const Exception& e)
    {
      m_context << "#### Could not rename '" << IO::pathAsQString(sourcePath) << "' to '"
                << IO::pathAsQString(targetPath) << "': " << e.what() << "\n";
      throw;
    }
  }
  catch (const Exception&)
  {
    emit error();
  }
}

void CompilationRenameFileTaskRunner::doTerminate() {}

CompilationDeleteFilesTaskRunner::CompilationDeleteFilesTaskRunner(
  CompilationContext& context, Model::CompilationDeleteFiles task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationDeleteFilesTaskRunner::~CompilationDeleteFilesTaskRunner() = default;

void CompilationDeleteFilesTaskRunner::doExecute()
{
  emit start();

  try
  {
    const auto targetPath = IO::Path{interpolate(m_task.targetSpec)};

    const auto targetDirPath = targetPath.deleteLastComponent();
    const auto targetPathMatcher =
      IO::makeFilenamePathMatcher(targetPath.lastComponent().string());

    try
    {
      const auto targetPaths = IO::Disk::find(targetDirPath, targetPathMatcher);
      const auto targetStrs = kdl::vec_transform(
        targetPaths, [](const auto& path) { return "'" + path.string() + "'"; });
      const auto targetListQStr = QString::fromStdString(kdl::str_join(targetStrs, ", "));
      m_context << "#### Deleting: " << targetListQStr << "\n";
      if (!m_context.test())
      {
        IO::Disk::deleteFiles(targetDirPath, targetPathMatcher);
      }
      emit end();
    }
    catch (const Exception& e)
    {
      m_context << "#### Could not delete '" << IO::pathAsQString(targetPath)
                << "': " << e.what() << "\n";
      throw;
    }
  }
  catch (const Exception&)
  {
    emit error();
  }
}

void CompilationDeleteFilesTaskRunner::doTerminate() {}

CompilationRunToolTaskRunner::CompilationRunToolTaskRunner(
  CompilationContext& context, Model::CompilationRunTool task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationRunToolTaskRunner::~CompilationRunToolTaskRunner() = default;

void CompilationRunToolTaskRunner::doExecute()
{
  startProcess();
}

void CompilationRunToolTaskRunner::doTerminate()
{
  if (m_process)
  {
    disconnect(
      m_process,
      &QProcess::errorOccurred,
      this,
      &CompilationRunToolTaskRunner::processErrorOccurred);
    disconnect(
      m_process,
      QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      this,
      &CompilationRunToolTaskRunner::processFinished);
    m_process->kill();
    m_context << "\n\n#### Terminated\n";
  }
}

void CompilationRunToolTaskRunner::startProcess()
{
  assert(m_process == nullptr);

  emit start();
  try
  {
    const auto workDir = m_context.variableValue(CompilationVariableNames::WORK_DIR_PATH);
    const auto cmd = this->cmd();

    m_context << "#### Executing '" << QString::fromStdString(cmd) << "'\n";

    if (!m_context.test())
    {
      m_process = new QProcess{this};
      connect(
        m_process,
        &QProcess::errorOccurred,
        this,
        &CompilationRunToolTaskRunner::processErrorOccurred);
      connect(
        m_process,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &CompilationRunToolTaskRunner::processFinished);
      connect(
        m_process,
        &QProcess::readyReadStandardError,
        this,
        &CompilationRunToolTaskRunner::processReadyReadStandardError);
      connect(
        m_process,
        &QProcess::readyReadStandardOutput,
        this,
        &CompilationRunToolTaskRunner::processReadyReadStandardOutput);

      m_process->setWorkingDirectory(QString::fromStdString(workDir));
      m_process->start(QString::fromStdString(cmd));
      if (!m_process->waitForStarted())
      {
        emit error();
      }
    }
    else
    {
      emit end();
    }
  }
  catch (const Exception&)
  {
    emit error();
  }
}

std::string CompilationRunToolTaskRunner::cmd()
{
  const auto toolPath = IO::Path{interpolate(m_task.toolSpec)};
  const auto parameters = interpolate(m_task.parameterSpec);
  if (parameters.empty())
  {
    return "\"" + toolPath.string() + "\"";
  }
  else if (toolPath.empty())
  {
    return "";
  }
  else
  {
    return "\"" + toolPath.string() + "\" " + parameters;
  }
}

void CompilationRunToolTaskRunner::processErrorOccurred(
  const QProcess::ProcessError processError)
{
  m_context << "#### Error '"
            << QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(processError)
            << "' occurred when communicating with process\n\n";
  emit error();
}

void CompilationRunToolTaskRunner::processFinished(
  const int exitCode, const QProcess::ExitStatus /* exitStatus */)
{
  m_context << "#### Finished with exit status " << exitCode << "\n\n";
  emit end();
}

void CompilationRunToolTaskRunner::processReadyReadStandardError()
{
  if (m_process != nullptr)
  {
    const QByteArray bytes = m_process->readAllStandardError();
    m_context << QString::fromLocal8Bit(bytes);
  }
}

void CompilationRunToolTaskRunner::processReadyReadStandardOutput()
{
  if (m_process != nullptr)
  {
    const QByteArray bytes = m_process->readAllStandardOutput();
    m_context << QString::fromLocal8Bit(bytes);
  }
}

CompilationRunner::CompilationRunner(
  std::unique_ptr<CompilationContext> context,
  const Model::CompilationProfile& profile,
  QObject* parent)
  : QObject{parent}
  , m_context{std::move(context)}
  , m_taskRunners{createTaskRunners(*m_context, profile)}
  , m_currentTask{std::end(m_taskRunners)}
{
}

CompilationRunner::~CompilationRunner() = default;

CompilationRunner::TaskRunnerList CompilationRunner::createTaskRunners(
  CompilationContext& context, const Model::CompilationProfile& profile)
{
  auto result = TaskRunnerList{};
  for (const auto& task : profile.tasks)
  {
    std::visit(
      kdl::overload(
        [&](const Model::CompilationExportMap& exportMap) {
          if (exportMap.enabled)
          {
            result.push_back(
              std::make_unique<CompilationExportMapTaskRunner>(context, exportMap));
          }
        },
        [&](const Model::CompilationCopyFiles& copyFiles) {
          if (copyFiles.enabled)
          {
            result.push_back(
              std::make_unique<CompilationCopyFilesTaskRunner>(context, copyFiles));
          }
        },
        [&](const Model::CompilationRenameFile& renameFile) {
          if (renameFile.enabled)
          {
            result.push_back(
              std::make_unique<CompilationRenameFileTaskRunner>(context, renameFile));
          }
        },
        [&](const Model::CompilationDeleteFiles& deleteFiles) {
          if (deleteFiles.enabled)
          {
            result.push_back(
              std::make_unique<CompilationDeleteFilesTaskRunner>(context, deleteFiles));
          }
        },
        [&](const Model::CompilationRunTool& runTool) {
          if (runTool.enabled)
          {
            result.push_back(
              std::make_unique<CompilationRunToolTaskRunner>(context, runTool));
          }
        }),
      task);
  }
  return result;
}

void CompilationRunner::execute()
{
  assert(!running());

  m_currentTask = std::begin(m_taskRunners);
  if (m_currentTask == std::end(m_taskRunners))
  {
    emit compilationEnded();
    return;
  }
  bindEvents(*m_currentTask->get());

  emit compilationStarted();

  const auto workDir = QString::fromStdString(
    m_context->variableValue(CompilationVariableNames::WORK_DIR_PATH));
  if (!QDir{workDir}.exists())
  {
    *m_context << "#### Error: working directory '" << workDir << "' does not exist\n";
  }
  else
  {
    *m_context << "#### Using working directory '" << workDir << "'\n";
  }
  m_currentTask->get()->execute();
}

void CompilationRunner::terminate()
{
  assert(running());
  unbindEvents(*m_currentTask->get());
  m_currentTask->get()->terminate();
  m_currentTask = std::end(m_taskRunners);

  emit compilationEnded();
}

bool CompilationRunner::running() const
{
  return m_currentTask != std::end(m_taskRunners);
}

void CompilationRunner::bindEvents(CompilationTaskRunner& runner) const
{
  connect(&runner, &CompilationTaskRunner::error, this, &CompilationRunner::taskError);
  connect(&runner, &CompilationTaskRunner::end, this, &CompilationRunner::taskEnd);
}

void CompilationRunner::unbindEvents(CompilationTaskRunner& runner) const
{
  runner.disconnect(this);
}

void CompilationRunner::taskError()
{
  if (running())
  {
    unbindEvents(*m_currentTask->get());
    m_currentTask = std::end(m_taskRunners);
    emit compilationEnded();
  }
}

void CompilationRunner::taskEnd()
{
  if (running())
  {
    unbindEvents(*m_currentTask->get());
    ++m_currentTask;
    if (m_currentTask != std::end(m_taskRunners))
    {
      bindEvents(*m_currentTask->get());
      m_currentTask->get()->execute();
    }
    else
    {
      emit compilationEnded();
    }
  }
}
} // namespace View
} // namespace TrenchBroom
