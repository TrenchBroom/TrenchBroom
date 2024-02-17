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

#include "GameEngineConfigWriter.h"

#include "EL/Value.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

#include "kdl/vector_utils.h"

#include <ostream>

namespace TrenchBroom
{
namespace IO
{
GameEngineConfigWriter::GameEngineConfigWriter(
  const Model::GameEngineConfig& config, std::ostream& stream)
  : m_config(config)
  , m_stream(stream)
{
  assert(!m_stream.bad());
}

void GameEngineConfigWriter::writeConfig()
{
  m_stream << EL::Value{EL::MapType{
    {"version", EL::Value{1.0}},
    {"profiles", writeProfiles(m_config)},
  }} << "\n";
}

EL::Value GameEngineConfigWriter::writeProfiles(
  const Model::GameEngineConfig& config) const
{
  return EL::Value{kdl::vec_transform(
    config.profiles, [&](const auto& profile) { return writeProfile(profile); })};
}

EL::Value GameEngineConfigWriter::writeProfile(
  const Model::GameEngineProfile& profile) const
{
  return EL::Value{EL::MapType{
    {"name", EL::Value{profile.name}},
    {"path", EL::Value{profile.path.string()}},
    {"parameters", EL::Value{profile.parameterSpec}},
  }};
}
} // namespace IO
} // namespace TrenchBroom
