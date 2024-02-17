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

#include "FloatType.h"
#include "Model/Validator.h"

#include "vecmath/bbox.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class WorldNode;
class Game;
class Node;

class SoftMapBoundsValidator : public Validator
{
private:
  std::weak_ptr<Game> m_game;
  const WorldNode& m_world;

public:
  explicit SoftMapBoundsValidator(std::weak_ptr<Game> game, const WorldNode& world);

private:
  void doValidate(
    EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const override;
  void doValidate(
    BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const override;
  void doValidate(
    PatchNode& patchNode, std::vector<std::unique_ptr<Issue>>& issues) const override;
};
} // namespace Model
} // namespace TrenchBroom
