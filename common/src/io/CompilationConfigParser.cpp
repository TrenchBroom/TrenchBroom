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

#include "kdl/ranges/to.h"

#include <fmt/format.h>

#include <ranges>
#include <string>

namespace tb::io
{
namespace
{

mdl::CompilationExportMap parseExportTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.contains(context, "enabled")
                         ? value.at(context, "enabled").booleanValue(context)
                         : true;
  return {
    enabled,
    value.at(context, "target").stringValue(context),
  };
}

mdl::CompilationCopyFiles parseCopyTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.contains(context, "enabled")
                         ? value.at(context, "enabled").booleanValue(context)
                         : true;
  return {
    enabled,
    value.at(context, "source").stringValue(context),
    value.at(context, "target").stringValue(context),
  };
}

mdl::CompilationRenameFile parseRenameTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.contains(context, "enabled")
                         ? value.at(context, "enabled").booleanValue(context)
                         : true;
  return {
    enabled,
    value.at(context, "source").stringValue(context),
    value.at(context, "target").stringValue(context),
  };
}

mdl::CompilationDeleteFiles parseDeleteTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.contains(context, "enabled")
                         ? value.at(context, "enabled").booleanValue(context)
                         : true;
  return {
    enabled,
    value.at(context, "target").stringValue(context),
  };
}

mdl::CompilationRunTool parseToolTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.contains(context, "enabled")
                         ? value.at(context, "enabled").booleanValue(context)
                         : true;
  const auto treatNonZeroResultCodeAsError =
    value.contains(context, "treatNonZeroResultCodeAsError")
      ? value.at(context, "treatNonZeroResultCodeAsError").booleanValue(context)
      : false;

  return {
    enabled,
    value.at(context, "tool").stringValue(context),
    value.at(context, "parameters").stringValue(context),
    treatNonZeroResultCodeAsError,
  };
}

mdl::CompilationTask parseTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto typeName = value.at(context, "type").stringValue(context);

  if (typeName == "export")
  {
    return parseExportTask(context, value);
  }
  if (typeName == "copy")
  {
    return parseCopyTask(context, value);
  }
  if (typeName == "rename")
  {
    return parseRenameTask(context, value);
  }
  if (typeName == "delete")
  {
    return parseDeleteTask(context, value);
  }
  if (typeName == "tool")
  {
    return parseToolTask(context, value);
  }

  throw ParserException{fmt::format("Unknown compilation task type '{}'", typeName)};
}

std::vector<mdl::CompilationTask> parseTasks(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& taskValue) {
           return parseTask(context, taskValue);
         })
         | kdl::ranges::to<std::vector>();
}

mdl::CompilationProfile parseProfile(
  const el::EvaluationContext& context, const el::Value& value)
{
  return {
    value.at(context, "name").stringValue(context),
    value.at(context, "workdir").stringValue(context),
    parseTasks(context, value.at(context, "tasks")),
  };
}

std::vector<mdl::CompilationProfile> parseProfiles(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& profileValue) {
           return parseProfile(context, profileValue);
         })
         | kdl::ranges::to<std::vector>();
}

Result<mdl::CompilationConfig> parseCompilationConfig(
  el::EvaluationContext& context, const el::ExpressionNode& expression)
{
  try
  {
    const auto root = expression.evaluate(context);
    if (const auto version = root.at(context, "version").numberValue(context);
        version != 1.0)
    {
      return Error{fmt::format("Unsupported compilation config version {}", version)};
    }

    return mdl::CompilationConfig{parseProfiles(context, root.at(context, "profiles"))};
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

} // namespace

CompilationConfigParser::CompilationConfigParser(
  const std::string_view str, std::filesystem::path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Result<mdl::CompilationConfig> CompilationConfigParser::parse()
{
  return parseConfigFile() | kdl::and_then([&](const auto& expression) {
           return el::withEvaluationContext(
             [&](auto& context) { return parseCompilationConfig(context, expression); });
         });
}

} // namespace tb::io
