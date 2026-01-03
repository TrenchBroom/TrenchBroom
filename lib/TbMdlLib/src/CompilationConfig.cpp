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

#include "mdl/CompilationConfig.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

namespace tb::mdl
{
namespace
{

el::Value toValue(const CompilationTask& task)
{
  return el::Value{std::visit(
    kdl::overload(
      [](const CompilationExportMap& exportMap) {
        auto map = el::MapType{};
        if (!exportMap.enabled)
        {
          map["enabled"] = el::Value{false};
        }
        map["type"] = el::Value{"export"};
        map["target"] = el::Value{exportMap.targetSpec};
        return map;
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
        return map;
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
        return map;
      },
      [](const CompilationDeleteFiles& deleteFiles) {
        auto map = el::MapType{};
        if (!deleteFiles.enabled)
        {
          map["enabled"] = el::Value{false};
        }
        map["type"] = el::Value{"delete"};
        map["target"] = el::Value{deleteFiles.targetSpec};
        return map;
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
        return map;
      }),
    task)};
}

el::Value toValue(const std::vector<CompilationTask>& tasks)
{
  return el::Value{
    tasks | std::views::transform([](const auto& task) { return toValue(task); })
    | kdl::ranges::to<std::vector>()};
}

el::Value toValue(const CompilationProfile& profile)
{
  return el::Value{el::MapType{
    {"name", el::Value{profile.name}},
    {"workdir", el::Value{profile.workDirSpec}},
    {"tasks", toValue(profile.tasks)},
  }};
}

el::Value toValue(const std::vector<CompilationProfile>& profiles)
{
  return el::Value{
    profiles
    | std::views::transform([&](const auto& profile) { return toValue(profile); })
    | kdl::ranges::to<std::vector>()};
}
} // namespace

kdl_reflect_impl(CompilationConfig);

el::Value toValue(const CompilationConfig& compilationConfig)
{
  return el::Value{el::MapType{
    {"version", el::Value{1.0}},
    {"profiles", toValue(compilationConfig.profiles)},
  }};
}

} // namespace tb::mdl
