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
CompilationConfigParser::CompilationConfigParser(std::string_view str, const Path& path)
  : ConfigParserBase(std::move(str), path)
{
}

Model::CompilationConfig CompilationConfigParser::parse()
{
  const EL::Value root = parseConfigFile().evaluate(EL::EvaluationContext());
  expectType(root, EL::ValueType::Map);

  expectStructure(root, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

  const EL::NumberType version = root["version"].numberValue();
  unused(version);
  assert(version == 1.0);

  auto profiles = parseProfiles(root["profiles"]);
  return Model::CompilationConfig(std::move(profiles));
}

std::vector<std::unique_ptr<Model::CompilationProfile>> CompilationConfigParser::
  parseProfiles(const EL::Value& value) const
{
  std::vector<std::unique_ptr<Model::CompilationProfile>> result;
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

  const std::string name = value["name"].stringValue();
  const std::string workdir = value["workdir"].stringValue();
  auto tasks = parseTasks(value["tasks"]);

  return std::make_unique<Model::CompilationProfile>(name, workdir, std::move(tasks));
}

std::vector<std::unique_ptr<Model::CompilationTask>> CompilationConfigParser::parseTasks(
  const EL::Value& value) const
{
  std::vector<std::unique_ptr<Model::CompilationTask>> result;
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
  const std::string type = value["type"].stringValue();

  if (type == "export")
  {
    return parseExportTask(value);
  }
  else if (type == "copy")
  {
    return parseCopyTask(value);
  }
  else if (type == "delete")
  {
    return parseDeleteTask(value);
  }
  else if (type == "tool")
  {
    return parseToolTask(value);
  }
  else
  {
    throw ParserException("Unknown compilation task type '" + type + "'");
  }
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseExportTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");
  const bool enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  const std::string target = value["target"].stringValue();
  return std::make_unique<Model::CompilationExportMap>(enabled, target);
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseCopyTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const bool enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  const std::string source = value["source"].stringValue();
  const std::string target = value["target"].stringValue();

  return std::make_unique<Model::CompilationCopyFiles>(enabled, source, target);
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseDeleteTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const bool enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  const std::string target = value["target"].stringValue();

  return std::make_unique<Model::CompilationDeleteFiles>(enabled, target);
}

std::unique_ptr<Model::CompilationTask> CompilationConfigParser::parseToolTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'tool': 'String', 'parameters': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const bool enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  const std::string tool = value["tool"].stringValue();
  const std::string parameters = value["parameters"].stringValue();

  return std::make_unique<Model::CompilationRunTool>(enabled, tool, parameters);
}
} // namespace IO
} // namespace TrenchBroom
