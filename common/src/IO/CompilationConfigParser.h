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

#include "EL/EL_Forward.h"
#include "IO/ConfigParserBase.h"
#include "Macros.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
class Path;

class CompilationConfigParser : public ConfigParserBase
{
public:
  explicit CompilationConfigParser(std::string_view str, Path path = Path{""});

  Model::CompilationConfig parse();

private:
  std::vector<Model::CompilationProfile> parseProfiles(const EL::Value& value) const;
  Model::CompilationProfile parseProfile(const EL::Value& value) const;
  std::vector<Model::CompilationTask> parseTasks(const EL::Value& value) const;
  Model::CompilationTask parseTask(const EL::Value& value) const;
  Model::CompilationExportMap parseExportTask(const EL::Value& value) const;
  Model::CompilationCopyFiles parseCopyTask(const EL::Value& value) const;
  Model::CompilationDeleteFiles parseDeleteTask(const EL::Value& value) const;
  Model::CompilationRunTool parseToolTask(const EL::Value& value) const;

  deleteCopyAndMove(CompilationConfigParser);
};
} // namespace IO
} // namespace TrenchBroom
