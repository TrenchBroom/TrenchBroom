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

#include "CompilationConfigWriter.h"

#include "EL/Types.h"
#include "EL/Value.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <ostream>

namespace TrenchBroom
{
namespace IO
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
  auto map = EL::MapType{};
  map["version"] = EL::Value{1.0};
  map["profiles"] = writeProfiles(m_config);
  m_stream << EL::Value{std::move(map)} << "\n";
}

EL::Value CompilationConfigWriter::writeProfiles(
  const Model::CompilationConfig& config) const
{
  auto array = EL::ArrayType{};
  for (size_t i = 0; i < config.profileCount(); ++i)
  {
    const auto* profile = config.profile(i);
    array.push_back(writeProfile(*profile));
  }

  return EL::Value{std::move(array)};
}

EL::Value CompilationConfigWriter::writeProfile(
  const Model::CompilationProfile& profile) const
{
  auto map = EL::MapType{};
  map["name"] = EL::Value{profile.name()};
  map["workdir"] = EL::Value{profile.workDirSpec()};
  map["tasks"] = writeTasks(profile);
  return EL::Value{std::move(map)};
}

EL::Value CompilationConfigWriter::writeTasks(
  const Model::CompilationProfile& profile) const
{
  return EL::Value{kdl::vec_transform(profile.tasks(), [](const auto& task) {
    return std::visit(
      kdl::overload(
        [](const Model::CompilationExportMap& exportMap) {
          auto map = EL::MapType{};
          if (!exportMap.enabled)
          {
            map["enabled"] = EL::Value{false};
          }
          map["type"] = EL::Value{"export"};
          map["target"] = EL::Value{exportMap.targetSpec};
          return EL::Value{std::move(map)};
        },
        [](const Model::CompilationCopyFiles& copyFiles) {
          auto map = EL::MapType{};
          if (!copyFiles.enabled)
          {
            map["enabled"] = EL::Value{false};
          }
          map["type"] = EL::Value{"copy"};
          map["source"] = EL::Value{copyFiles.sourceSpec};
          map["target"] = EL::Value{copyFiles.targetSpec};
          return EL::Value{std::move(map)};
        },
        [](const Model::CompilationDeleteFiles& deleteFiles) {
          auto map = EL::MapType{};
          if (!deleteFiles.enabled)
          {
            map["enabled"] = EL::Value{false};
          }
          map["type"] = EL::Value{"delete"};
          map["target"] = EL::Value{deleteFiles.targetSpec};
          return EL::Value{std::move(map)};
        },
        [](const Model::CompilationRunTool& exportMap) {
          auto map = EL::MapType{};
          if (!exportMap.enabled)
          {
            map["enabled"] = EL::Value{false};
          }
          map["type"] = EL::Value{"tool"};
          map["tool"] = EL::Value{exportMap.toolSpec};
          map["parameters"] = EL::Value{exportMap.parameterSpec};
          return EL::Value{std::move(map)};
        }),
      task);
  })};
}
} // namespace IO
} // namespace TrenchBroom
