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

#include "CompilationRunner.h"

#include <QDir>
#include <QMetaEnum>
#include <QProcess>
#include <QtGlobal>

#include "Exceptions.h"
#include "io/DiskIO.h"
#include "io/ExportOptions.h"
#include "io/PathInfo.h"
#include "io/PathMatcher.h"
#include "io/PathQt.h"
#include "io/TraversalMode.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"
#include "ui/CompilationContext.h"
#include "ui/CompilationVariables.h"
#include "ui/MapDocument.h" // IWYU pragma: keep

#include "kdl/functional.h"
#include "kdl/overload.h"
#include "kdl/result_fold.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>
#include <ranges>
#include <string>

namespace tb::ui
{

namespace
{

Result<std::string> workDir(const CompilationContext& context)
{
  try
  {
    return context.variableValue(CompilationVariableNames::WORK_DIR_PATH);
  }
  catch (const Exception& e)
  {
    return Error{e.what()};
  }
}

} // namespace

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

Result<std::string> CompilationTaskRunner::interpolate(const std::string& spec) const
{
  try
  {
    return m_context.interpolate(spec);
  }
  catch (const Exception& e)
  {
    return Error{
      fmt::format("Could not interpolate expression '{}': {}", spec, e.what())};
  }
}

CompilationExportMapTaskRunner::CompilationExportMapTaskRunner(
  CompilationContext& context, mdl::CompilationExportMap task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationExportMapTaskRunner::~CompilationExportMapTaskRunner() = default;

void CompilationExportMapTaskRunner::doExecute()
{
  emit start();

  interpolate(m_task.targetSpec).and_then([&](const auto& interpolated) {
    const auto targetPath = std::filesystem::path{interpolated};
    m_context << "#### Exporting map file '" << io::pathAsQString(targetPath) << "'\n";

    if (!m_context.test())
    {
      return io::Disk::createDirectory(targetPath.parent_path())
             | kdl::and_then([&](auto) {
                 const auto options = io::MapExportOptions{targetPath};
                 const auto document = m_context.document();
                 return document->exportDocumentAs(options);
               });
    }
    return Result<void>{};
  }) | kdl::transform([&]() { emit end(); })
    | kdl::transform_error([&](auto e) {
        m_context << "#### Export failed: " << QString::fromStdString(e.msg) << "\n";
        emit error();
      });
}

void CompilationExportMapTaskRunner::doTerminate() {}

CompilationCopyFilesTaskRunner::CompilationCopyFilesTaskRunner(
  CompilationContext& context, mdl::CompilationCopyFiles task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationCopyFilesTaskRunner::~CompilationCopyFilesTaskRunner() = default;

void CompilationCopyFilesTaskRunner::doExecute()
{
  emit start();

  interpolate(m_task.sourceSpec)
      .join(interpolate(m_task.targetSpec))
      .and_then([&](const auto& interpolatedSource, const auto& interpolatedTarget) {
        const auto sourcePath = std::filesystem::path{interpolatedSource};
        const auto targetPath = std::filesystem::path{interpolatedTarget};

        const auto sourceDirPath = sourcePath.parent_path();
        const auto sourcePathMatcher = kdl::lift_and(
          io::makePathInfoPathMatcher({io::PathInfo::File}),
          io::makeFilenamePathMatcher(sourcePath.filename().string()));

        return io::Disk::find(sourceDirPath, io::TraversalMode::Flat, sourcePathMatcher)
               | kdl::and_then([&](const auto& pathsToCopy) {
                   const auto pathStrsToCopy = kdl::vec_transform(
                     pathsToCopy,
                     [](const auto& path) { return "'" + path.string() + "'"; });

                   m_context << "#### Copying to '" << io::pathAsQString(targetPath)
                             << "/': "
                             << QString::fromStdString(
                                  kdl::str_join(pathStrsToCopy, ", "))
                             << "\n";
                   if (!m_context.test())
                   {
                     return io::Disk::createDirectory(targetPath)
                            | kdl::and_then([&](auto) {
                                return kdl::vec_transform(
                                         pathsToCopy,
                                         [&](const auto& pathToCopy) {
                                           return io::Disk::copyFile(
                                             pathToCopy, targetPath);
                                         })
                                       | kdl::fold;
                              });
                   }
                   return Result<void>{};
                 });
      })
    | kdl::transform([&]() { emit end(); }) | kdl::transform_error([&](auto e) {
        m_context << "#### Copy failed: " << QString::fromStdString(e.msg) << "\n";
        emit error();
      });
}

void CompilationCopyFilesTaskRunner::doTerminate() {}

CompilationRenameFileTaskRunner::CompilationRenameFileTaskRunner(
  CompilationContext& context, mdl::CompilationRenameFile task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationRenameFileTaskRunner::~CompilationRenameFileTaskRunner() = default;

void CompilationRenameFileTaskRunner::doExecute()
{
  emit start();

  interpolate(m_task.sourceSpec)
      .join(interpolate(m_task.targetSpec))
      .and_then([&](const auto& interpolatedSource, const auto& interpolatedTarget) {
        const auto sourcePath = std::filesystem::path{interpolatedSource};
        const auto targetPath = std::filesystem::path{interpolatedTarget};

        m_context << "#### Renaming '" << io::pathAsQString(sourcePath) << "' to '"
                  << io::pathAsQString(targetPath) << "'\n";
        if (!m_context.test())
        {
          return io::Disk::createDirectory(targetPath.parent_path())
                 | kdl::and_then(
                   [&](auto) { return io::Disk::moveFile(sourcePath, targetPath); });
        }
        return Result<void>{};
      })
    | kdl::transform([&]() { emit end(); }) | kdl::transform_error([&](auto e) {
        m_context << "#### Rename failed: " << QString::fromStdString(e.msg) << "\n";
        emit error();
      });
}

void CompilationRenameFileTaskRunner::doTerminate() {}

CompilationDeleteFilesTaskRunner::CompilationDeleteFilesTaskRunner(
  CompilationContext& context, mdl::CompilationDeleteFiles task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationDeleteFilesTaskRunner::~CompilationDeleteFilesTaskRunner() = default;

void CompilationDeleteFilesTaskRunner::doExecute()
{
  emit start();

  interpolate(m_task.targetSpec).and_then([&](const auto& interpolated) {
    const auto targetPath = std::filesystem::path{interpolated};

    const auto targetDirPath = targetPath.parent_path();
    const auto targetPathMatcher = kdl::lift_and(
      io::makePathInfoPathMatcher({io::PathInfo::File}),
      io::makeFilenamePathMatcher(targetPath.filename().string()));

    return io::Disk::find(targetDirPath, io::TraversalMode::Recursive, targetPathMatcher)
           | kdl::transform([&](const auto& pathsToDelete) {
               const auto pathStrsToDelete = kdl::vec_transform(
                 pathsToDelete,
                 [](const auto& path) { return "'" + path.string() + "'"; });
               m_context << "#### Deleting: "
                         << QString::fromStdString(kdl::str_join(pathStrsToDelete, ", "))
                         << "\n";

               if (!m_context.test())
               {
                 return kdl::vec_transform(pathsToDelete, io::Disk::deleteFile)
                        | kdl::fold;
               }
               return Result<std::vector<bool>>{std::vector<bool>{}};
             });
  }) | kdl::transform([&](auto) { emit end(); })
    | kdl::transform_error([&](auto e) {
        m_context << "#### Delete failed: " << QString::fromStdString(e.msg) << "\n";
        emit error();
      });
}

void CompilationDeleteFilesTaskRunner::doTerminate() {}

CompilationRunToolTaskRunner::CompilationRunToolTaskRunner(
  CompilationContext& context, mdl::CompilationRunTool task)
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
  workDir(m_context)
      .join(program())
      .join(parameters())
      .and_then(
        [&](const auto& workDir, const auto& program, const auto& parameters)
          -> Result<void> {
          const auto programStr = QString::fromStdString(program);
          const auto parameterStrs =
            parameters | std::views::transform([](const auto& p) {
              return QString::fromStdString(p);
            });
          const auto parameterStrList =
            QStringList{parameterStrs.begin(), parameterStrs.end()};

          m_context << "#### Executing '" << programStr << " "
                    << parameterStrList.join(" ") << "'\n";

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

            m_process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
            m_process->setWorkingDirectory(QString::fromStdString(workDir));
            m_process->setArguments(parameterStrList);
            m_process->setProgram(programStr);
            m_process->start();
            if (!m_process->waitForStarted())
            {
              return Error{"Failed to start process"};
            }
          }
          return Result<void>{};
        })
    | kdl::transform([&]() {
        if (m_context.test())
        {
          emit end();
        }
      })
    | kdl::transform_error([&](auto e) {
        m_context << "#### Execution failed: " << QString::fromStdString(e.msg) << "\n";
        emit error();
      });
}

Result<std::string> CompilationRunToolTaskRunner::program() const
{
  return interpolate(m_task.toolSpec);
}

Result<std::vector<std::string>> CompilationRunToolTaskRunner::parameters() const
{
  return interpolate(m_task.parameterSpec).transform([](const auto& parameters) {
    return kdl::str_split(parameters, " ");
  });
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
  const int exitCode, const QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
  case QProcess::NormalExit:
    m_context << "#### Finished with exit code " << exitCode << "\n\n";
    if (exitCode == 0 || !m_task.treatNonZeroResultCodeAsError)
    {
      emit end();
    }
    else
    {
      emit error();
    }
    break;
  case QProcess::CrashExit:
    m_context << "#### Crashed with exit code " << exitCode << "\n\n";
    emit error();
    break;
  }
}

void CompilationRunToolTaskRunner::processReadyReadStandardError()
{
  if (m_process)
  {
    const QByteArray bytes = m_process->readAllStandardError();
    m_context << QString::fromLocal8Bit(bytes);
  }
}

void CompilationRunToolTaskRunner::processReadyReadStandardOutput()
{
  if (m_process)
  {
    const QByteArray bytes = m_process->readAllStandardOutput();
    m_context << QString::fromLocal8Bit(bytes);
  }
}

CompilationRunner::CompilationRunner(
  CompilationContext context, const mdl::CompilationProfile& profile, QObject* parent)
  : QObject{parent}
  , m_context{std::move(context)}
  , m_taskRunners{createTaskRunners(m_context, profile)}
  , m_currentTask{std::end(m_taskRunners)}
{
}

CompilationRunner::~CompilationRunner() = default;

CompilationRunner::TaskRunnerList CompilationRunner::createTaskRunners(
  CompilationContext& context, const mdl::CompilationProfile& profile)
{
  auto result = TaskRunnerList{};
  for (const auto& task : profile.tasks)
  {
    std::visit(
      kdl::overload(
        [&](const mdl::CompilationExportMap& exportMap) {
          if (exportMap.enabled)
          {
            result.push_back(
              std::make_unique<CompilationExportMapTaskRunner>(context, exportMap));
          }
        },
        [&](const mdl::CompilationCopyFiles& copyFiles) {
          if (copyFiles.enabled)
          {
            result.push_back(
              std::make_unique<CompilationCopyFilesTaskRunner>(context, copyFiles));
          }
        },
        [&](const mdl::CompilationRenameFile& renameFile) {
          if (renameFile.enabled)
          {
            result.push_back(
              std::make_unique<CompilationRenameFileTaskRunner>(context, renameFile));
          }
        },
        [&](const mdl::CompilationDeleteFiles& deleteFiles) {
          if (deleteFiles.enabled)
          {
            result.push_back(
              std::make_unique<CompilationDeleteFilesTaskRunner>(context, deleteFiles));
          }
        },
        [&](const mdl::CompilationRunTool& runTool) {
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
    return;
  }
  bindEvents(*m_currentTask->get());

  emit compilationStarted();

  workDir(m_context)
    .transform([&](const auto& workDir) {
      const auto workDirQStr = QString::fromStdString(workDir);
      if (QDir{workDirQStr}.exists())
      {
        m_context << "#### Using working directory '" << workDirQStr << "'\n";
      }
      else
      {
        m_context << "#### Error: working directory '" << workDirQStr
                  << "' does not exist\n";
      }
      m_currentTask->get()->execute();
    })
    .transform_error([&](const auto& e) {
      m_context << "#### Error: Could not get determine working directory: "
                << QString::fromStdString(e.msg) << "\n";
    });
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

} // namespace tb::ui
