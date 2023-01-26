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

#include "GameEngineConfigParser.h"

#include "EL/EvaluationContext.h"
#include "EL/Expression.h"
#include "EL/Value.h"
#include "Macros.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
GameEngineConfigParser::GameEngineConfigParser(std::string_view str, Path path)
  : ConfigParserBase{std::move(str), std::move(path)}
{
}

Model::GameEngineConfig GameEngineConfigParser::parse()
{
  const auto root = parseConfigFile().evaluate(EL::EvaluationContext());
  expectType(root, EL::ValueType::Map);

  expectStructure(root, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

  const auto version = root["version"].numberValue();
  unused(version);
  assert(version == 1.0);

  return {parseProfiles(root["profiles"])};
}

std::vector<Model::GameEngineProfile> GameEngineConfigParser::parseProfiles(
  const EL::Value& value) const
{
  auto result = std::vector<Model::GameEngineProfile>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value[i]));
  }
  return result;
}

Model::GameEngineProfile GameEngineConfigParser::parseProfile(
  const EL::Value& value) const
{
  expectStructure(
    value, "[ {'name': 'String', 'path': 'String'}, { 'parameters': 'String' } ]");

  return {
    value["name"].stringValue(),
    Path{value["path"].stringValue()},
    value["parameters"].stringValue()};
}
} // namespace IO
} // namespace TrenchBroom
