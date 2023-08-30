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
CompilationConfigParser::CompilationConfigParser(
  const std::string_view str, std::filesystem::path path)
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

std::vector<Model::CompilationProfile> CompilationConfigParser::parseProfiles(
  const EL::Value& value) const
{
  auto result = std::vector<Model::CompilationProfile>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value[i]));
  }
  return result;
}

Model::CompilationProfile CompilationConfigParser::parseProfile(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

  return {
    value["name"].stringValue(),
    value["workdir"].stringValue(),
    parseTasks(value["tasks"])};
}

std::vector<Model::CompilationTask> CompilationConfigParser::parseTasks(
  const EL::Value& value) const
{
  auto result = std::vector<Model::CompilationTask>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseTask(value[i]));
  }
  return result;
}

Model::CompilationTask CompilationConfigParser::parseTask(const EL::Value& value) const
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
  if (typeName == "rename")
  {
    return parseRenameTask(value);
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

Model::CompilationExportMap CompilationConfigParser::parseExportTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

Model::CompilationCopyFiles CompilationConfigParser::parseCopyTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

Model::CompilationRenameFile CompilationConfigParser::parseRenameTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

Model::CompilationDeleteFiles CompilationConfigParser::parseDeleteTask(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

Model::CompilationRunTool CompilationConfigParser::parseToolTask(
  const EL::Value& value) const
{
  expectStructure(
    value,
    "[ {'type': 'String', 'tool': 'String', 'parameters': 'String'}, { 'enabled': "
    "'Boolean', 'treatNonZeroResultCodeAsError': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  const auto treatNonZeroResultCodeAsError =
    value.contains("treatNonZeroResultCodeAsError")
      ? value["treatNonZeroResultCodeAsError"].booleanValue()
      : false;

  return {
    enabled,
    value["tool"].stringValue(),
    value["parameters"].stringValue(),
    treatNonZeroResultCodeAsError};
}
} // namespace IO
} // namespace TrenchBroom
