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

#pragma once

#include "EL/EL_Forward.h"
#include "IO/ELParser.h"
#include "IO/Path.h"

#include <string>

namespace TrenchBroom::IO
{

class ConfigParserBase
{
private:
  ELParser m_parser;

protected:
  Path m_path;

protected:
  explicit ConfigParserBase(std::string_view str, Path path = Path{""});

public:
  virtual ~ConfigParserBase();

protected:
  EL::Expression parseConfigFile();
};

void expectType(const EL::Value& value, EL::ValueType type);
void expectStructure(const EL::Value& value, const std::string& structure);
void expectMapEntry(const EL::Value& value, const std::string& key, EL::ValueType type);
} // namespace TrenchBroom::IO
