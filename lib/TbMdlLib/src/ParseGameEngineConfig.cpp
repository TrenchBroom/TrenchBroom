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

#include "mdl/ParseGameEngineConfig.h"

#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "el/Value.h"
#include "mdl/GameEngineConfig.h"
#include "mdl/GameEngineProfile.h"

#include "kd/ranges/to.h"

#include <fmt/format.h>

#include <ranges>
#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{

GameEngineProfile toProfile(const el::EvaluationContext& context, const el::Value& value)
{
  return {
    value.at(context, "name").stringValue(context),
    std::filesystem::path{value.at(context, "path").stringValue(context)},
    value.at(context, "parameters").stringValue(context),
  };
}

std::vector<GameEngineProfile> toProfiles(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& profileValue) {
           return toProfile(context, profileValue);
         })
         | kdl::ranges::to<std::vector>();
}

Result<GameEngineConfig> toGameEngineConfig(
  el::EvaluationContext& context, const el::ExpressionNode& expression)
{
  const auto root = expression.evaluate(context);

  if (const auto version = root.at(context, "version").numberValue(context);
      version != 1.0)
  {
    return Error{fmt::format("Unsupported game engine config version {}", version)};
  }

  return GameEngineConfig{toProfiles(context, root.at(context, "profiles"))};
}

} // namespace

Result<GameEngineConfig> parseGameEngineConfig(const std::string_view str)
{
  return el::parseExpression(el::ParseMode::Strict, str)
         | kdl::and_then([&](const auto& expression) {
             return el::withEvaluationContext(
               [&](auto& context) { return toGameEngineConfig(context, expression); });
           });
}

} // namespace tb::mdl
