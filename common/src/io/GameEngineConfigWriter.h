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

#pragma once

#include "Macros.h"
#include "el/Forward.h"

#include <iosfwd>

namespace tb
{
namespace mdl
{
struct GameEngineConfig;
struct GameEngineProfile;
} // namespace mdl

namespace io
{

class GameEngineConfigWriter
{
private:
  const mdl::GameEngineConfig& m_config;
  std::ostream& m_stream;

public:
  GameEngineConfigWriter(const mdl::GameEngineConfig& config, std::ostream& stream);

  void writeConfig();

private:
  el::Value writeProfiles(const mdl::GameEngineConfig& config) const;
  el::Value writeProfile(const mdl::GameEngineProfile& profile) const;

  deleteCopyAndMove(GameEngineConfigWriter);
};

} // namespace io
} // namespace tb
