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

#include "ConfigParserBase.h"

#include "Exceptions.h"
#include "el/EvaluationContext.h"
#include "el/EvaluationTrace.h"
#include "el/Expression.h"
#include "el/Value.h"

#include <fmt/format.h>

#include <string>

namespace tb::io
{

ConfigParserBase::ConfigParserBase(const std::string_view str, std::filesystem::path path)
  : m_parser{ELParser::Mode::Strict, str}
  , m_path{std::move(path)}
{
}

ConfigParserBase::~ConfigParserBase() = default;

el::ExpressionNode ConfigParserBase::parseConfigFile()
{
  return m_parser.parse();
}

void expectType(
  const el::Value& value, const el::EvaluationTrace& trace, const el::ValueType type)
{
  if (value.type() != type)
  {
    throw ParserException{
      *trace.getLocation(value),
      fmt::format(
        "Expected value of type '{}', but got type '{}'",
        el::typeName(type),
        value.typeName())};
  }
}

void expectStructure(
  const el::Value& value, const el::EvaluationTrace& trace, const std::string& structure)
{
  auto parser = ELParser{ELParser::Mode::Strict, structure};
  const auto expected = parser.parse().evaluate(el::EvaluationContext());
  assert(expected.type() == el::ValueType::Array);

  const auto mandatory = expected[0];
  assert(mandatory.type() == el::ValueType::Map);

  const auto optional = expected[1];
  assert(optional.type() == el::ValueType::Map);

  // Are all mandatory keys present?
  for (const auto& key : mandatory.keys())
  {
    const auto typeName = mandatory[key].stringValue();
    if (typeName != "*")
    {
      const auto type = el::typeForName(typeName);
      expectMapEntry(value, trace, key, type);
    }
  }
}

void expectMapEntry(
  const el::Value& value,
  const el::EvaluationTrace& trace,
  const std::string& key,
  const el::ValueType type)
{
  const auto& map = value.mapValue();
  const auto it = map.find(key);
  if (it == std::end(map))
  {
    throw ParserException{
      *trace.getLocation(value), fmt::format("Expected map entry '{}'", key)};
  }
  expectType(it->second, trace, type);
}

} // namespace tb::io
