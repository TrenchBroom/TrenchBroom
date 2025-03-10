/*
 Copyright (C) 2025 Kristian Duske

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

#include "LaunchGameEngine.h"

#include <QProcess>
#include <QString>
#include <QStringList>

#include "Exceptions.h"
#include "el/EvaluationContext.h"
#include "el/Interpolator.h"
#include "io/PathQt.h"
#include "mdl/GameEngineProfile.h"

#include "kdl/cmd_utils.h"

#include <fmt/format.h>

namespace tb::ui
{
namespace
{

auto arguments(const mdl::GameEngineProfile& profile, const el::VariableStore& variables)
{
  const auto parameters =
    el::interpolate(profile.parameterSpec, el::EvaluationContext{variables});

  auto result = QStringList{};
  for (const auto& parameter : kdl::cmd_parse_args(parameters))
  {
    result.push_back(QString::fromStdString(parameter));
  }
  return result;
}

} // namespace

Result<void> launchGameEngineProfile(
  const mdl::GameEngineProfile& profile,
  const el::VariableStore& variables,
  const std::optional<std::filesystem::path>& logFilePath)
{
  try
  {
    const auto workDir = io::pathAsQString(profile.path.parent_path());
    const auto engineArguments = arguments(profile, variables);

    auto process = QProcess{};
    process.setWorkingDirectory(workDir);

    if (logFilePath)
    {
      const auto qLogFilePath = io::pathAsQString(*logFilePath);
      process.setStandardOutputFile(qLogFilePath);
      process.setStandardErrorFile(qLogFilePath);
    }

    constexpr auto isMacOs =
#ifdef __APPLE__
      true;
#else
      false;
#endif

    if (profile.path.extension() == ".app" && isMacOs)
    {
      // We have to launch apps via the 'open' command so that we can properly pass
      // parameters.
      auto launchArguments = QStringList{};
      launchArguments << "-a" << io::pathAsQString(profile.path) << "--args";
      launchArguments.append(engineArguments);

      process.setProgram("/usr/bin/open");
      process.setArguments(launchArguments);
    }
    else
    {
      process.setProgram(io::pathAsQString(profile.path));
      process.setArguments(engineArguments);
    }

    if (!process.startDetached())
    {
      return Result<void>{Error{fmt::format(
        "Failed to launch game engine: {}", process.errorString().toStdString())}};
    }
  }
  catch (const Exception& e)
  {
    return Result<void>{Error{fmt::format("Failed to launch game engine: {}", e.what())}};
  }

  return Result<void>{};
}

} // namespace tb::ui
