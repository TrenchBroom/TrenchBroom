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
#include "Macros.h"

#include <iosfwd>

namespace TrenchBroom
{
namespace Model
{
class GameEngineConfig;
class GameEngineProfile;
} // namespace Model

namespace IO
{
class GameEngineConfigWriter
{
private:
  const Model::GameEngineConfig& m_config;
  std::ostream& m_stream;

public:
  GameEngineConfigWriter(const Model::GameEngineConfig& config, std::ostream& stream);

  void writeConfig();

private:
  EL::Value writeProfiles(const Model::GameEngineConfig& config) const;
  EL::Value writeProfile(const Model::GameEngineProfile* profile) const;

  deleteCopyAndMove(GameEngineConfigWriter);
};
} // namespace IO
} // namespace TrenchBroom
