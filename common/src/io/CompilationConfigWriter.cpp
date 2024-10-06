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

#include "CompilationConfigWriter.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "el/Types.h"
#include "el/Value.h"

#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include <cassert>
#include <ostream>

namespace tb::io
{

CompilationConfigWriter::CompilationConfigWriter(
  const Model::CompilationConfig& config, std::ostream& stream)
  : m_config{config}
  , m_stream{stream}
{
  assert(!m_stream.bad());
}

void CompilationConfigWriter::writeConfig()
{
  auto map = el::MapType{};
  map["version"] = el::Value{1.0};
  map["profiles"] = writeProfiles(m_config);
  m_stream << el::Value{std::move(map)} << "\n";
}

el::Value CompilationConfigWriter::writeProfiles(
  const Model::CompilationConfig& config) const
{
  return el::Value{kdl::vec_transform(
    config.profiles, [&](const auto& profile) { return writeProfile(profile); })};
}

el::Value CompilationConfigWriter::writeProfile(
  const Model::CompilationProfile& profile) const
{
  return el::Value{el::MapType{
    {"name", el::Value{profile.name}},
    {"workdir", el::Value{profile.workDirSpec}},
    {"tasks", writeTasks(profile)},
  }};
}

el::Value CompilationConfigWriter::writeTasks(
  const Model::CompilationProfile& profile) const
{
  return el::Value{kdl::vec_transform(profile.tasks, [](const auto& task) {
    return std::visit(
      kdl::overload(
        [](const Model::CompilationExportMap& exportMap) {
          auto map = el::MapType{};
          if (!exportMap.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"export"};
          map["target"] = el::Value{exportMap.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const Model::CompilationCopyFiles& copyFiles) {
          auto map = el::MapType{};
          if (!copyFiles.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"copy"};
          map["source"] = el::Value{copyFiles.sourceSpec};
          map["target"] = el::Value{copyFiles.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const Model::CompilationRenameFile& renameFile) {
          auto map = el::MapType{};
          if (!renameFile.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"rename"};
          map["source"] = el::Value{renameFile.sourceSpec};
          map["target"] = el::Value{renameFile.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const Model::CompilationDeleteFiles& deleteFiles) {
          auto map = el::MapType{};
          if (!deleteFiles.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"delete"};
          map["target"] = el::Value{deleteFiles.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const Model::CompilationRunTool& runTool) {
          auto map = el::MapType{};
          if (!runTool.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          if (runTool.treatNonZeroResultCodeAsError)
          {
            map["treatNonZeroResultCodeAsError"] = el::Value{true};
          }
          map["type"] = el::Value{"tool"};
          map["tool"] = el::Value{runTool.toolSpec};
          map["parameters"] = el::Value{runTool.parameterSpec};
          return el::Value{std::move(map)};
        }),
      task);
  })};
}

} // namespace tb::io
