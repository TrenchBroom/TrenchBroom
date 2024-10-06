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

#include "CompilationConfigParser.h"

#include "el/EvaluationContext.h"
#include "el/EvaluationTrace.h"
#include "el/Expression.h"
#include "el/Value.h"
#include "mdl/CompilationConfig.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"

#include <fmt/format.h>

#include <string>

namespace tb::io
{
namespace
{

mdl::CompilationExportMap parseExportTask(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

mdl::CompilationCopyFiles parseCopyTask(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

mdl::CompilationRenameFile parseRenameTask(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["source"].stringValue(), value["target"].stringValue()};
}

mdl::CompilationDeleteFiles parseDeleteTask(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled = value.contains("enabled") ? value["enabled"].booleanValue() : true;
  return {enabled, value["target"].stringValue()};
}

mdl::CompilationRunTool parseToolTask(
  const el::Value& value, const el::EvaluationTrace& trace)
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

mdl::CompilationTask parseTask(const el::Value& value, const el::EvaluationTrace& trace)
{
  expectMapEntry(value, trace, "type", el::ValueType::String);
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

std::vector<mdl::CompilationTask> parseTasks(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  auto result = std::vector<mdl::CompilationTask>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseTask(value[i], trace));
  }
  return result;
}

mdl::CompilationProfile parseProfile(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

  return {
    value["name"].stringValue(),
    value["workdir"].stringValue(),
    parseTasks(value["tasks"], trace)};
}

std::vector<mdl::CompilationProfile> parseProfiles(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  auto result = std::vector<mdl::CompilationProfile>{};
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

mdl::CompilationConfig CompilationConfigParser::parse()
{
  const auto context = el::EvaluationContext{};
  auto trace = el::EvaluationTrace{};

  const auto root = parseConfigFile().evaluate(context, trace);
  expectType(root, trace, el::ValueType::Map);

  expectStructure(root, trace, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

  const auto version = root["version"].numberValue();
  unused(version);
  assert(version == 1.0);

  return mdl::CompilationConfig{parseProfiles(root["profiles"], trace)};
}

} // namespace tb::io
