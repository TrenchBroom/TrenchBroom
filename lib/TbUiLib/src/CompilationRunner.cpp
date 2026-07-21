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

#include "ui/CompilationRunner.h"

#include <QDir>
#include <QMetaEnum>
#include <QProcess>
#include <QtGlobal>

#include "el/Exceptions.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "fs/PathMatcher.h"
#include "fs/TraversalMode.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"
#include "mdl/ExportOptions.h"
#include "mdl/GameEngineProfile.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "ui/CompilationContext.h"
#include "ui/CompilationVariables.h"
#include "ui/LaunchGameEngine.h"
#include "ui/QPathUtils.h"

#include "kd/cmd_utils.h"
#include "kd/contracts.h"
#include "kd/functional.h"
#include "kd/overload.h"
#include "kd/path_utils.h"
#include "kd/ranges/to.h"
#include "kd/result_fold.h"
#include "kd/string_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

#include <algorithm>
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
  catch (const el::Exception& e)
  {
    return Error{e.what()};
  }
}

auto makeAbsolute(const std::filesystem::path& path, const std::filesystem::path& workDir)
{
  return path.is_absolute() ? path : workDir / path;
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
  catch (const el::Exception& e)
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


  workDir(m_context)
      .join(interpolate(m_task.targetSpec))
      .and_then([&](const auto& workDir, const auto& interpolated) {
        const auto targetPath = makeAbsolute(kdl::parse_path(interpolated), workDir);
        m_context << "#### Exporting map file '" << pathAsQString(targetPath) << "'\n";

        auto entityToAdd = m_task.entityToAdd | kdl::optional_transform([&](auto entity) {
                             const auto position = m_context.camera().position();
                             const auto yaw = vm::to_degrees(m_context.camera().yaw());

                             entity.addOrUpdateProperty(
                               mdl::EntityPropertyKeys::Origin,
                               fmt::format("{}", fmt::streamed(position)));
                             entity.addOrUpdateProperty(
                               mdl::EntityPropertyKeys::Angle, fmt::format("{}", yaw));

                             return entity;
                           });

        if (!m_context.test())
        {
          return fs::Disk::createDirectory(targetPath.parent_path())
                 | kdl::and_then([&](auto) {
                     const auto options = mdl::MapExportOptions{
                       targetPath,
                       m_task.stripTbProperties,
                       m_task.stripEntityPattern,
                       std::move(entityToAdd),
                     };
                     return m_context.map().exportAs(options);
                   });
        }
        return Result<void>{};
      })
    | kdl::transform([&]() { emit end(); }) | kdl::transform_error([&](auto e) {
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

  workDir(m_context)
      .join(interpolate(m_task.sourceSpec))
      .join(interpolate(m_task.targetSpec))
      .and_then([&](
                  const auto& workDir,
                  const auto& interpolatedSource,
                  const auto& interpolatedTarget) {
        const auto sourcePath =
          makeAbsolute(kdl::parse_path(interpolatedSource), workDir);
        const auto targetPath =
          makeAbsolute(kdl::parse_path(interpolatedTarget), workDir);

        const auto sourceDirPath = sourcePath.parent_path();
        const auto sourcePathMatcher = kdl::logical_and(
          fs::makePathInfoPathMatcher({fs::PathInfo::File}),
          fs::makeFilenamePathMatcher(sourcePath.filename().string()));

        return fs::Disk::find(sourceDirPath, fs::TraversalMode::Flat, sourcePathMatcher)
               | kdl::and_then([&](const auto& pathsToCopy) {
                   const auto pathStrsToCopy =
                     pathsToCopy | std::views::transform([](const auto& path) {
                       return fmt::format("{}", path);
                     })
                     | kdl::ranges::to<std::vector>();

                   m_context << "#### Copying to '" << pathAsQString(targetPath) << "/': "
                             << QString::fromStdString(
                                  kdl::str_join(pathStrsToCopy, ", "))
                             << "\n";
                   if (!m_context.test())
                   {
                     return fs::Disk::createDirectory(targetPath)
                            | kdl::and_then([&](auto) {
                                return pathsToCopy
                                       | std::views::transform(
                                         [&](const auto& pathToCopy) {
                                           return fs::Disk::copyFile(
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

  workDir(m_context)
      .join(interpolate(m_task.sourceSpec))
      .join(interpolate(m_task.targetSpec))
      .and_then([&](
                  const auto& workDir,
                  const auto& interpolatedSource,
                  const auto& interpolatedTarget) {
        const auto sourcePath =
          makeAbsolute(kdl::parse_path(interpolatedSource), workDir);
        const auto targetPath =
          makeAbsolute(kdl::parse_path(interpolatedTarget), workDir);

        m_context << "#### Renaming '" << pathAsQString(sourcePath) << "' to '"
                  << pathAsQString(targetPath) << "'\n";
        if (!m_context.test())
        {
          return fs::Disk::createDirectory(targetPath.parent_path())
                 | kdl::and_then(
                   [&](auto) { return fs::Disk::moveFile(sourcePath, targetPath); });
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

  workDir(m_context)
      .join(interpolate(m_task.targetSpec))
      .and_then([&](const auto& workDir, const auto& interpolated) {
        const auto targetPath = makeAbsolute(kdl::parse_path(interpolated), workDir);

        const auto targetDirPath = targetPath.parent_path();
        const auto targetPathMatcher = kdl::logical_and(
          fs::makePathInfoPathMatcher({fs::PathInfo::File}),
          fs::makeFilenamePathMatcher(targetPath.filename().string()));

        return fs::Disk::find(
                 targetDirPath, fs::TraversalMode::Recursive, targetPathMatcher)
               | kdl::transform([&](const auto& pathsToDelete) {
                   const auto pathStrsToDelete =
                     pathsToDelete | std::views::transform([](const auto& path) {
                       return fmt::format("{}", path);
                     })
                     | kdl::ranges::to<std::vector>();

                   m_context << "#### Deleting: "
                             << QString::fromStdString(
                                  kdl::str_join(pathStrsToDelete, ", "))
                             << "\n";

                   if (!m_context.test())
                   {
                     return pathsToDelete | std::views::transform(fs::Disk::deleteFile)
                            | kdl::fold;
                   }
                   return Result<std::vector<bool>>{std::vector<bool>{}};
                 });
      })
    | kdl::transform([&](auto) { emit end(); }) | kdl::transform_error([&](auto e) {
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
  contract_pre(m_process == nullptr);

  emit start();
  workDir(m_context)
      .join(program())
      .join(parameters())
      .and_then(
        [&](const auto& workDir, const auto& program, const auto& parameters)
          -> Result<void> {
          const auto programStr = pathAsQString(kdl::parse_path(program));
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
    return kdl::cmd_parse_args(parameters);
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

CompilationLaunchEngineTaskRunner::CompilationLaunchEngineTaskRunner(
  CompilationContext& context, mdl::CompilationLaunchEngine task)
  : CompilationTaskRunner{context}
  , m_task{std::move(task)}
{
}

CompilationLaunchEngineTaskRunner::~CompilationLaunchEngineTaskRunner() = default;

void CompilationLaunchEngineTaskRunner::doExecute()
{
  emit start();

  const auto fail = [&](const QString& message) {
    m_context << "#### Launch failed: " << message << "\n";
    if (m_task.treatLaunchFailureAsError)
    {
      emit error();
    }
    else
    {
      m_context << "#### Continuing despite launch failure\n";
      emit end();
    }
  };

  if (m_task.engineProfileId.empty())
  {
    fail("Engine profile is not set");
    return;
  }

  const auto& engineProfiles = m_context.map().gameInfo().gameEngineConfig.profiles;
  const auto profileIt = std::ranges::find_if(engineProfiles, [&](const auto& profile) {
    return profile.id == m_task.engineProfileId;
  });

  if (profileIt == std::end(engineProfiles))
  {
    fail(
      QString{"Engine profile '"} + QString::fromStdString(m_task.engineProfileId)
      + "' was not found");
    return;
  }

  const auto& profile = *profileIt;
  m_context << "#### Launching engine profile '" << QString::fromStdString(profile.name)
            << "' at '" << pathAsQString(profile.path) << "'\n";

  if (m_context.test())
  {
    emit end();
    return;
  }

  launchGameEngineProfile(profile, LaunchGameEngineVariables{m_context.map()})
    | kdl::transform([&]() {
        m_context << "#### Launched\n";
        emit end();
      })
    | kdl::transform_error([&](auto e) { fail(QString::fromStdString(e.msg)); });
}

void CompilationLaunchEngineTaskRunner::doTerminate() {}

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
        },
        [&](const mdl::CompilationLaunchEngine& launchEngine) {
          if (launchEngine.enabled)
          {
            result.push_back(
              std::make_unique<CompilationLaunchEngineTaskRunner>(context, launchEngine));
          }
        }),
      task);
  }
  return result;
}

void CompilationRunner::execute()
{
  contract_pre(!running());

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
  contract_pre(running());

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
