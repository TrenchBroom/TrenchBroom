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

#include "mdl/CompilationConfigWriter.h"

#include "el/Types.h"
#include "el/Value.h"
#include "mdl/CompilationConfig.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/ranges/to.h"

#include <ostream>

namespace tb::mdl
{

CompilationConfigWriter::CompilationConfigWriter(
  const CompilationConfig& config, std::ostream& stream)
  : m_config{config}
  , m_stream{stream}
{
  contract_pre(m_stream.good());
}

void CompilationConfigWriter::writeConfig()
{
  auto map = el::MapType{};
  map["version"] = el::Value{1.0};
  map["profiles"] = writeProfiles(m_config);
  m_stream << el::Value{std::move(map)} << "\n";
}

el::Value CompilationConfigWriter::writeProfiles(const CompilationConfig& config) const
{
  return el::Value{
    config.profiles
    | std::views::transform([&](const auto& profile) { return writeProfile(profile); })
    | kdl::ranges::to<std::vector>()};
}

el::Value CompilationConfigWriter::writeProfile(const CompilationProfile& profile) const
{
  return el::Value{el::MapType{
    {"name", el::Value{profile.name}},
    {"workdir", el::Value{profile.workDirSpec}},
    {"tasks", writeTasks(profile)},
  }};
}

el::Value CompilationConfigWriter::writeTasks(const CompilationProfile& profile) const
{
  const auto writeTask = [](const auto& task) {
    return std::visit(
      kdl::overload(
        [](const CompilationExportMap& exportMap) {
          auto map = el::MapType{};
          if (!exportMap.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"export"};
          map["target"] = el::Value{exportMap.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const CompilationCopyFiles& copyFiles) {
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
        [](const CompilationRenameFile& renameFile) {
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
        [](const CompilationDeleteFiles& deleteFiles) {
          auto map = el::MapType{};
          if (!deleteFiles.enabled)
          {
            map["enabled"] = el::Value{false};
          }
          map["type"] = el::Value{"delete"};
          map["target"] = el::Value{deleteFiles.targetSpec};
          return el::Value{std::move(map)};
        },
        [](const CompilationRunTool& runTool) {
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
  };

  return el::Value{
    profile.tasks | std::views::transform(writeTask) | kdl::ranges::to<std::vector>()};
}

} // namespace tb::mdl
