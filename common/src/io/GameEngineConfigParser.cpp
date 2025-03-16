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
#include "el/EvaluationTrace.h"
#include "el/Value.h"
#include "mdl/GameEngineConfig.h"
#include "mdl/GameEngineProfile.h"

#include <string>
#include <vector>

namespace tb::io
{
namespace
{

mdl::GameEngineProfile parseProfile(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  expectStructure(
    value, trace, "[ {'name': 'String', 'path': 'String'}, { 'parameters': 'String' } ]");

  return {
    value["name"].stringValue(),
    std::filesystem::path{value["path"].stringValue()},
    value["parameters"].stringValue()};
}

std::vector<mdl::GameEngineProfile> parseProfiles(
  const el::Value& value, const el::EvaluationTrace& trace)
{
  auto result = std::vector<mdl::GameEngineProfile>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    result.push_back(parseProfile(value[i], trace));
  }
  return result;
}

} // namespace

GameEngineConfigParser::GameEngineConfigParser(
  const std::string_view str, std::filesystem::path path)
  : ConfigParserBase{str, std::move(path)}
{
}

Result<mdl::GameEngineConfig> GameEngineConfigParser::parse()
{
  return parseConfigFile()
         | kdl::and_then([&](const auto& expression) -> Result<mdl::GameEngineConfig> {
             try
             {
               const auto context = el::EvaluationContext{};
               auto trace = el::EvaluationTrace{};

               const auto root = expression.evaluate(context, trace);
               expectType(root, trace, el::ValueType::Map);

               expectStructure(
                 root, trace, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

               const auto version = root["version"].numberValue();
               unused(version);
               assert(version == 1.0);

               return mdl::GameEngineConfig{parseProfiles(root["profiles"], trace)};
             }
             catch (const Exception& e)
             {
               return Error{e.what()};
             }
           });
}

} // namespace tb::io
