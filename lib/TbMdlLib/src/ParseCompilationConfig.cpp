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

#include "mdl/ParseCompilationConfig.h"

#include "ParserException.h"
#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "el/Value.h"
#include "mdl/CompilationConfig.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"

#include "kd/ranges/to.h"

#include <fmt/format.h>

#include <ranges>
#include <string>

namespace tb::mdl
{
namespace
{

CompilationExportMap toExportTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled =
    value.atOrDefault(context, "enabled", el::Value{true}).booleanValue(context);

  const auto stripTbProperties =
    value.atOrDefault(context, "stripTbProperties", el::Value{false})
      .booleanValue(context);

  return {
    enabled,
    stripTbProperties,
    value.at(context, "target").stringValue(context),
  };
}

CompilationCopyFiles toCopyTask(
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

CompilationRenameFile toRenameTask(
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

CompilationDeleteFiles toDeleteTask(
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

CompilationRunTool toToolTask(
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

CompilationTask toTask(const el::EvaluationContext& context, const el::Value& value)
{
  const auto typeName = value.at(context, "type").stringValue(context);

  if (typeName == "export")
  {
    return toExportTask(context, value);
  }
  if (typeName == "copy")
  {
    return toCopyTask(context, value);
  }
  if (typeName == "rename")
  {
    return toRenameTask(context, value);
  }
  if (typeName == "delete")
  {
    return toDeleteTask(context, value);
  }
  if (typeName == "tool")
  {
    return toToolTask(context, value);
  }

  throw ParserException{fmt::format("Unknown compilation task type '{}'", typeName)};
}

std::vector<CompilationTask> toTasks(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& taskValue) {
           return toTask(context, taskValue);
         })
         | kdl::ranges::to<std::vector>();
}

CompilationProfile toProfile(const el::EvaluationContext& context, const el::Value& value)
{
  return {
    value.at(context, "name").stringValue(context),
    value.at(context, "workdir").stringValue(context),
    toTasks(context, value.at(context, "tasks")),
  };
}

std::vector<CompilationProfile> toProfiles(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& profileValue) {
           return toProfile(context, profileValue);
         })
         | kdl::ranges::to<std::vector>();
}

Result<CompilationConfig> toCompilationConfig(
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

    return CompilationConfig{toProfiles(context, root.at(context, "profiles"))};
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

} // namespace


Result<CompilationConfig> parseCompilationConfig(const std::string_view str)
{
  return el::parseExpression(el::ParseMode::Strict, str)
         | kdl::and_then([&](const auto& expression) {
             return el::withEvaluationContext(
               [&](auto& context) { return toCompilationConfig(context, expression); });
           });
}

} // namespace tb::mdl
