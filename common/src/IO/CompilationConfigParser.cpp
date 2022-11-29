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

#include "CompilationConfigParser.h"

#include "EL/EvaluationContext.h"
#include "EL/Expression.h"
#include "EL/Value.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <kdl/vector_utils.h>

#include <string>

namespace TrenchBroom
{
namespace IO
{
CompilationConfigParser::CompilationConfigParser(const std::string_view str, Path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Model::CompilationConfig CompilationConfigParser::parse()
{
  const auto root = parseConfigFile().evaluate(EL::EvaluationContext());
  expectType(root, EL::ValueType::Map);

  expectStructure(root, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

  const auto version = root["version"].numberValue();
  unused(version);
  assert(version == 1.0);

  return Model::CompilationConfig{parseProfiles(root["profiles"])};
}

std::vector<std::unique_ptr<Model::CompilationProfile>> CompilationConfigParser::
  parseProfiles(const EL::Value& value) const
{
  auto result = std::vector<std::unique_ptr<Model::CompilationProfile>>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value[i]));
  }
  return result;
}

std::unique_ptr<Model::CompilationProfile> CompilationConfigParser::parseProfile(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

  auto name = value["name"].stringValue();
  auto workdir = value["workdir"].stringValue();
  auto tasks = parseTasks(value["tasks"]);

  return std::make_unique<Model::CompilationProfile>(
    std::move(name), std::move(workdir), std::move(tasks));
}

std::vector<std::unique_ptr<Model::CompilationTask>> CompilationConfigParser::parseTasks(
  const EL::Value& value) const
{
  auto result = std::vector<std::unique_ptr<Model::CompilationTask>>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseTask(value[i]));
  }
  return result;
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseTask(
  const EL::Value& value) const
{
  expectMapEntry(value, "type", EL::ValueType::String);
  const auto typeName = value["type"].stringValue();

  if (typeName == "export")
  {
    return parseExportTask(value);
  }
  if (typeName == "copy")
  {
    return parseCopyTask(value);
  }
  if (typeName == "delete")
  {
    return parseDeleteTask(value);
  }
  if (typeName == "tool")
  {
    return parseToolTask(value);
  }

  throw ParserException{"Unknown compilation task type '" + typeName + "'"};
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseExportTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  auto target = value["target"].stringValue();

  return std::make_unique<Model::CompilationExportMap>(enabled, std::move(target));
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseCopyTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  auto source = value["source"].stringValue();
  auto target = value["target"].stringValue();

  return std::make_unique<Model::CompilationCopyFiles>(
    enabled, std::move(source), std::move(target));
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseDeleteTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  auto target = value["target"].stringValue();

  return std::make_unique<Model::CompilationDeleteFiles>(enabled, std::move(target));
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseToolTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'tool': 'String', 'parameters': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  auto tool = value["tool"].stringValue();
  auto parameters = value["parameters"].stringValue();

  return std::make_unique<Model::CompilationRunTool>(
    enabled, std::move(tool), std::move(parameters));
}
} // namespace IO
} // namespace TrenchBroom
