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

#include "GameEngineConfigWriter.h"

#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "el/Value.h"

#include "kdl/vector_utils.h"

#include <ostream>

namespace tb::IO
{

GameEngineConfigWriter::GameEngineConfigWriter(
  const Model::GameEngineConfig& config, std::ostream& stream)
  : m_config{config}
  , m_stream{stream}
{
  assert(!m_stream.bad());
}

void GameEngineConfigWriter::writeConfig()
{
  m_stream << el::Value{el::MapType{
    {"version", el::Value{1.0}},
    {"profiles", writeProfiles(m_config)},
  }} << "\n";
}

el::Value GameEngineConfigWriter::writeProfiles(
  const Model::GameEngineConfig& config) const
{
  return el::Value{kdl::vec_transform(
    config.profiles, [&](const auto& profile) { return writeProfile(profile); })};
}

el::Value GameEngineConfigWriter::writeProfile(
  const Model::GameEngineProfile& profile) const
{
  return el::Value{el::MapType{
    {"name", el::Value{profile.name}},
    {"path", el::Value{profile.path.string()}},
    {"parameters", el::Value{profile.parameterSpec}},
  }};
}

} // namespace tb::IO
