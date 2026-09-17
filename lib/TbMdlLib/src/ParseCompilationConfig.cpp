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

#include "base/ParserException.h"
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

Entity toEntity(const el::EvaluationContext&, const el::Value& entityValue)
{
  auto entityProperties = entityValue.arrayValue()
                          | std::views::transform([&](const auto& propertyValue) {
                              const auto& map = propertyValue.mapValue();
                              return EntityProperty{
                                map.at("key").stringValue(),
                                map.at("value").stringValue(),
                              };
                            })
                          | kdl::ranges::to<std::vector>();

  return Entity{std::move(entityProperties)};
}

CompilationExportMap toExportTask(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto enabled = value.atOrDefault("enabled", el::Value{true}).booleanValue();

  const auto stripTbProperties =
    value.atOrDefault("stripTbProperties", el::Value{false}).booleanValue();

  auto stripEntityPattern =
    value.contains("stripEntityPattern")
      ? std::optional{value.at("stripEntityPattern").stringValue()}
      : std::nullopt;

  auto entityToAdd = value.contains("entityToAdd")
                       ? std::optional{toEntity(context, value.at("entityToAdd"))}
                       : std::nullopt;

  return {
    enabled,
    stripTbProperties,
    std::move(stripEntityPattern),
    std::move(entityToAdd),
    value.at("target").stringValue(),
  };
}

CompilationCopyFiles toCopyTask(const el::EvaluationContext&, const el::Value& value)
{
  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {
    enabled,
    value.at("source").stringValue(),
    value.at("target").stringValue(),
  };
}

CompilationRenameFile toRenameTask(const el::EvaluationContext&, const el::Value& value)
{
  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {
    enabled,
    value.at("source").stringValue(),
    value.at("target").stringValue(),
  };
}

CompilationDeleteFiles toDeleteTask(const el::EvaluationContext&, const el::Value& value)
{
  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  return {
    enabled,
    value.at("target").stringValue(),
  };
}

CompilationRunTool toToolTask(const el::EvaluationContext&, const el::Value& value)
{
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
    treatNonZeroResultCodeAsError,
  };
}

CompilationLaunchEngine toLaunchEngineTask(
  const el::EvaluationContext&, const el::Value& value)
{
  const auto enabled =
    value.contains("enabled") ? value.at("enabled").booleanValue() : true;
  const auto treatLaunchFailureAsError =
    value.contains("treatLaunchFailureAsError")
      ? value.at("treatLaunchFailureAsError").booleanValue()
      : false;

  return {
    enabled,
    value.at("engineProfileId").stringValue(),
    treatLaunchFailureAsError,
  };
}

CompilationTask toTask(const el::EvaluationContext& context, const el::Value& value)
{
  const auto typeName = value.at("type").stringValue();

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
  if (typeName == "launchEngine")
  {
    return toLaunchEngineTask(context, value);
  }

  throw ParserException{fmt::format("Unknown compilation task type '{}'", typeName)};
}

std::vector<CompilationTask> toTasks(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue() | std::views::transform([&](const auto& taskValue) {
           return toTask(context, taskValue);
         })
         | kdl::ranges::to<std::vector>();
}

CompilationProfile toProfile(const el::EvaluationContext& context, const el::Value& value)
{
  return {
    value.at("name").stringValue(),
    value.at("workdir").stringValue(),
    toTasks(context, value.at("tasks")),
  };
}

std::vector<CompilationProfile> toProfiles(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue() | std::views::transform([&](const auto& profileValue) {
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
    if (const auto version = root.at("version").numberValue(); version != 1.0)
    {
      return Error{fmt::format("Unsupported compilation config version {}", version)};
    }

    return CompilationConfig{toProfiles(context, root.at("profiles"))};
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
