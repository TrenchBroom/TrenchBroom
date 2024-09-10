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
#include "EL/EvaluationTrace.h"
#include "EL/ExpressionNode.h"
#include "EL/Value.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <string>

namespace TrenchBroom::IO
{
namespace
{

Model::CompilationExportMap parseExportTask(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

Model::CompilationCopyFiles parseCopyTask(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

Model::CompilationRenameFile parseRenameTask(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

Model::CompilationDeleteFiles parseDeleteTask(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

Model::CompilationRunTool parseToolTask(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
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

Model::CompilationTask parseTask(const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectMapEntry(value, trace, "type", EL::ValueType::String);
  const auto typeName = value["type"].stringValue();

  if (typeName == "export")
  {
    return parseExportTask(value, trace);
  }
  if (typeName == "copy")
  {
    return parseCopyTask(value, trace);
  }
  if (typeName == "rename")
  {
    return parseRenameTask(value, trace);
  }
  if (typeName == "delete")
  {
    return parseDeleteTask(value, trace);
  }
  if (typeName == "tool")
  {
    return parseToolTask(value, trace);
  }

  throw ParserException{fmt::format("Unknown compilation task type '{}'", typeName)};
}

std::vector<Model::CompilationTask> parseTasks(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  auto result = std::vector<Model::CompilationTask>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseTask(value[i], trace));
  }
  return result;
}

Model::CompilationProfile parseProfile(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

  return {
    value["name"].stringValue(),
    value["workdir"].stringValue(),
    parseTasks(value["tasks"], trace)};
}

std::vector<Model::CompilationProfile> parseProfiles(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  auto result = std::vector<Model::CompilationProfile>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value[i], trace));
  }
  return result;
}

} // namespace

CompilationConfigParser::CompilationConfigParser(
  const std::string_view str, std::filesystem::path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Model::CompilationConfig CompilationConfigParser::parse()
{
  const auto context = EL::EvaluationContext{};
  auto trace = EL::EvaluationTrace{};

  const auto root = parseConfigFile().evaluate(context, trace);
  expectType(root, trace, EL::ValueType::Map);

  expectStructure(root, trace, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

  const auto version = root["version"].numberValue();
  unused(version);
  assert(version == 1.0);

  return Model::CompilationConfig{parseProfiles(root["profiles"], trace)};
}

} // namespace TrenchBroom::IO
