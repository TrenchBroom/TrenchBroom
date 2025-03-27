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

#include "el/Expression.h"

#include <fmt/format.h>

namespace tb::io
{

ConfigParserBase::ConfigParserBase(const std::string_view str, std::filesystem::path path)
  : m_parser{ELParser::Mode::Strict, str}
  , m_path{std::move(path)}
{
}

ConfigParserBase::~ConfigParserBase() = default;

Result<el::ExpressionNode> ConfigParserBase::parseConfigFile()
{
  return m_parser.parse();
}

} // namespace tb::io
