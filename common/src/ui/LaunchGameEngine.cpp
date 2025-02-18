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

#include "el/EvaluationContext.h"
#include "el/Interpolator.h"
#include "io/PathQt.h"
#include "mdl/GameEngineProfile.h"

namespace tb::ui
{

Result<void> launchGameEngineProfile(
  const mdl::GameEngineProfile& profile, const el::VariableStore& variables)
{
  try
  {
    const auto parameters =
      el::interpolate(profile.parameterSpec, el::EvaluationContext{variables});

    const auto workDir = io::pathAsQString(profile.path.parent_path());

#ifdef __APPLE__
    // We have to launch apps via the 'open' command so that we can properly pass
    // parameters.
    const auto arguments = QStringList{
      "-a",
      io::pathAsQString(profile.path),
      "--args",
      QString::fromStdString(parameters)};

    if (!QProcess::startDetached("/usr/bin/open", arguments, workDir))
    {
      return Result<void>{Error{"Unknown error"}};
    }
#else
    const auto arguments = QStringList{QString::fromStdString(parameters)};
    if (!QProcess::startDetached(io::pathAsQString(profile.path), arguments, workDir))
    {
      return Result<void>{Error{"Unknown error"}};
    }
#endif
  }
  catch (const Exception& e)
  {
    return Result<void>{Error{e.what()}};
  }

  return Result<void>{};
}

} // namespace tb::ui
