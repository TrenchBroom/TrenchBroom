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

#include "Game.h"

#include "Assets/EntityDefinitionFileSpec.h"
#include "Error.h"
#include "IO/ExportOptions.h"
#include "Model/BrushFace.h"
#include "Model/GameFactory.h"
#include "Model/WorldNode.h"

#include "kdl/result.h"

#include <string>
#include <vector>

namespace TrenchBroom::Model
{

bool Game::isGamePathPreference(const std::filesystem::path& prefPath) const
{
  const GameFactory& gameFactory = GameFactory::instance();
  return gameFactory.isGamePathPreference(gameName(), prefPath);
}

} // namespace TrenchBroom::Model
