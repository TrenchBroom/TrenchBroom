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

namespace
{
class WriteCompilationTaskVisitor : public Model::ConstCompilationTaskVisitor
{
private:
  EL::ArrayType m_array;

public:
  EL::Value result() const { return EL::Value(m_array); }

public:
  void visit(const Model::CompilationExportMap& task) override
  {
    auto map = EL::MapType{};
    if (!task.enabled())
    {
      map["enabled"] = EL::Value{false};
    }
    map["type"] = EL::Value{"export"};
    map["target"] = EL::Value{task.targetSpec()};
    m_array.emplace_back(std::move(map));
  }

  void visit(const Model::CompilationCopyFiles& task) override
  {
    auto map = EL::MapType{};
    if (!task.enabled())
    {
      map["enabled"] = EL::Value{false};
    }
    map["type"] = EL::Value{"copy"};
    map["source"] = EL::Value{task.sourceSpec()};
    map["target"] = EL::Value{task.targetSpec()};
    m_array.emplace_back(std::move(map));
  }

  void visit(const Model::CompilationDeleteFiles& task) override
  {
    auto map = EL::MapType{};
    if (!task.enabled())
    {
      map["enabled"] = EL::Value{false};
    }
    map["type"] = EL::Value{"delete"};
    map["target"] = EL::Value{task.targetSpec()};
    m_array.emplace_back(std::move(map));
  }

  void visit(const Model::CompilationRunTool& task) override
  {
    auto map = EL::MapType{};
    if (!task.enabled())
    {
      map["enabled"] = EL::Value{false};
    }
    map["type"] = EL::Value{"tool"};
    map["tool"] = EL::Value{task.toolSpec()};
    map["parameters"] = EL::Value{task.parameterSpec()};
    m_array.emplace_back(std::move(map));
  }
};
} // namespace

EL::Value CompilationConfigWriter::writeTasks(
  const Model::CompilationProfile& profile) const
{
  auto visitor = WriteCompilationTaskVisitor{};
  profile.accept(visitor);
  return visitor.result();
}
} // namespace IO
} // namespace TrenchBroom
