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
#include "el/Value.h"
#include "io/ParserException.h"
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
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {enabled, value.at("target").stringValue()};
}

mdl::CompilationCopyFiles parseCopyTask(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {enabled, value.at("source").stringValue(), value.at("target").stringValue()};
}

mdl::CompilationRenameFile parseRenameTask(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    "[ {'type': 'String', 'source': 'String', 'target': 'String'}, { 'enabled': "
    "'Boolean' } ]");

  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {enabled, value.at("source").stringValue(), value.at("target").stringValue()};
}

mdl::CompilationDeleteFiles parseDeleteTask(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    "[ {'type': 'String', 'target': 'String'}, { 'enabled': 'Boolean' } ]");

  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {enabled, value.at("target").stringValue()};
}

mdl::CompilationRunTool parseToolTask(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    "[ {'type': 'String', 'tool': 'String', 'parameters': 'String'}, { 'enabled': "
    "'Boolean', 'treatNonZeroResultCodeAsError': 'Boolean' } ]");

  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  const auto treatNonZeroResultCodeAsError =
    value.contains("treatNonZeroResultCodeAsError")
      ? value.at("treatNonZeroResultCodeAsError").booleanValue()
      : false;

  return {
    enabled,
    value.at("tool").stringValue(),
    value.at("parameters").stringValue(),
    treatNonZeroResultCodeAsError};
}

mdl::CompilationTask parseTask(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectMapEntry(value, context, "type", el::ValueType::String);
  const auto typeName = value.at("type").stringValue();

  if (typeName == "export")
  {
    return parseExportTask(value, context);
  }
  if (typeName == "copy")
  {
    return parseCopyTask(value, context);
  }
  if (typeName == "rename")
  {
    return parseRenameTask(value, context);
  }
  if (typeName == "delete")
  {
    return parseDeleteTask(value, context);
  }
  if (typeName == "tool")
  {
    return parseToolTask(value, context);
  }

  throw ParserException{fmt::format("Unknown compilation task type '{}'", typeName)};
}

std::vector<mdl::CompilationTask> parseTasks(
  const el::Value& value, const el::EvaluationContext& context)
{
  auto result = std::vector<mdl::CompilationTask>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseTask(value.at(i), context));
  }
  return result;
}

mdl::CompilationProfile parseProfile(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value, context, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

  return {
    value.at("name").stringValue(),
    value.at("workdir").stringValue(),
    parseTasks(value.at("tasks"), context)};
}

std::vector<mdl::CompilationProfile> parseProfiles(
  const el::Value& value, const el::EvaluationContext& context)
{
  auto result = std::vector<mdl::CompilationProfile>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value.at(i), context));
  }
  return result;
}

} // namespace

CompilationConfigParser::CompilationConfigParser(
  const std::string_view str, std::filesystem::path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Result<mdl::CompilationConfig> CompilationConfigParser::parse()
{
  return parseConfigFile()
         | kdl::and_then([&](const auto& expression) -> Result<mdl::CompilationConfig> {
             try
             {
               auto context = el::EvaluationContext{};

               const auto root = expression.evaluate(context);
               expectType(root, context, el::ValueType::Map);

               expectStructure(
                 root, context, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

               const auto version = root.at("version").numberValue();
               unused(version);
               assert(version == 1.0);

               return mdl::CompilationConfig{parseProfiles(root.at("profiles"), context)};
             }
             catch (const Exception& e)
             {
               return Error{e.what()};
             }
           });
}

} // namespace tb::io
