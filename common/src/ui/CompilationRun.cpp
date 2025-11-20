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

#include "CompilationRun.h"

#include "el/Interpolate.h"
#include "mdl/CompilationProfile.h"
#include "ui/CompilationContext.h"
#include "ui/CompilationRunner.h"
#include "ui/CompilationVariables.h"
#include "ui/MapDocument.h"
#include "ui/TextOutputAdapter.h"

#include "kd/contracts.h"

#include <string>

namespace tb::ui
{

CompilationRun::~CompilationRun()
{
  if (running())
  {
    m_currentRun->terminate();
  }
}

bool CompilationRun::running() const
{
  return doIsRunning();
}

Result<void> CompilationRun::run(
  const mdl::CompilationProfile& profile, const mdl::Map& map, QTextEdit* currentOutput)
{
  return run(profile, map, currentOutput, false);
}

Result<void> CompilationRun::test(
  const mdl::CompilationProfile& profile, const mdl::Map& map, QTextEdit* currentOutput)
{
  return run(profile, map, currentOutput, true);
}

void CompilationRun::terminate()
{
  if (doIsRunning())
  {
    m_currentRun->terminate();
  }
}

bool CompilationRun::doIsRunning() const
{
  return m_currentRun != nullptr && m_currentRun->running();
}

Result<void> CompilationRun::run(
  const mdl::CompilationProfile& profile,
  const mdl::Map& map,
  QTextEdit* currentOutput,
  const bool test)
{
  contract_pre(!profile.tasks.empty());
  contract_pre(currentOutput != nullptr);
  contract_pre(!doIsRunning());

  cleanup();

  return buildWorkDir(profile, map) | kdl::transform([&](const auto& workDir) {
           auto variables = CompilationVariables{map, workDir};
           auto compilationContext =
             CompilationContext{map, variables, TextOutputAdapter{currentOutput}, test};
           m_currentRun =
             new CompilationRunner{std::move(compilationContext), profile, this};
           connect(
             m_currentRun,
             &CompilationRunner::compilationStarted,
             this,
             &CompilationRun::compilationStarted);
           connect(m_currentRun, &CompilationRunner::compilationEnded, this, [&]() {
             cleanup();
             emit compilationEnded();
           });
           m_currentRun->execute();
         });
}

Result<std::string> CompilationRun::buildWorkDir(
  const mdl::CompilationProfile& profile, const mdl::Map& map)
{
  return el::interpolate(CompilationWorkDirVariables{map}, profile.workDirSpec);
}

void CompilationRun::cleanup()
{
  if (m_currentRun)
  {
    // It's not safe to delete a CompilationRunner during execution of one of its
    // signals, so use deleteLater()
    m_currentRun->deleteLater();
    m_currentRun = nullptr;
  }
}

} // namespace tb::ui
