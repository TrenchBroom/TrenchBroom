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

#include "GameEngineConfigParser.h"

#include "Macros.h"
#include "el/EvaluationContext.h"
#include "el/Value.h"
#include "mdl/GameEngineConfig.h"
#include "mdl/GameEngineProfile.h"

#include "kdl/range_to_vector.h"

#include <ranges>
#include <string>
#include <vector>

namespace tb::io
{
namespace
{

mdl::GameEngineProfile parseProfile(
  const el::EvaluationContext& context, const el::Value& value)
{
  return {
    value.at(context, "name").stringValue(context),
    std::filesystem::path{value.at(context, "path").stringValue(context)},
    value.at(context, "parameters").stringValue(context),
  };
}

std::vector<mdl::GameEngineProfile> parseProfiles(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& profileValue) {
           return parseProfile(context, profileValue);
         })
         | kdl::to_vector;
}

Result<mdl::GameEngineConfig> parseGameEngineConfig(
  el::EvaluationContext& context, const el::ExpressionNode& expression)
{
  const auto root = expression.evaluate(context);

  if (const auto version = root.at(context, "version").numberValue(context);
      version != 1.0)
  {
    return Error{fmt::format("Unsupported game engine config version {}", version)};
  }

  return mdl::GameEngineConfig{parseProfiles(context, root.at(context, "profiles"))};
}

} // namespace

GameEngineConfigParser::GameEngineConfigParser(
  const std::string_view str, std::filesystem::path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Result<mdl::GameEngineConfig> GameEngineConfigParser::parse()
{
  return parseConfigFile() | kdl::and_then([&](const auto& expression) {
           return el::withEvaluationContext(
             [&](auto& context) { return parseGameEngineConfig(context, expression); });
         });
}

} // namespace tb::io
