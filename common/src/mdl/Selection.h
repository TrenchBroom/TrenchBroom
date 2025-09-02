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

#include "mdl/BrushFaceHandle.h"

#include "kdl/reflection_decl.h"

#include <vector>

namespace tb::mdl
{
class BrushNode;
class EntityNode;
class EntityNodeBase;
class GroupNode;
class LayerNode;
class Node;
class PatchNode;
class WorldNode;

struct Selection
{
  std::vector<Node*> nodes;
  std::vector<GroupNode*> groups;
  std::vector<EntityNode*> entities;
  std::vector<BrushNode*> brushes;
  std::vector<PatchNode*> patches;
  std::vector<BrushFaceHandle> brushFaces;

  std::vector<EntityNodeBase*> cachedAllEntities;
  std::vector<BrushNode*> cachedAllBrushes;
  std::vector<BrushFaceHandle> cachedAllBrushFaces;

  kdl_reflect_decl(Selection, nodes, groups, entities, brushes, patches, brushFaces);

  bool hasAny() const;
  bool hasNodes() const;
  bool hasGroups() const;
  bool hasOnlyGroups() const;
  bool hasEntities() const;
  bool hasOnlyEntities() const;
  bool hasBrushes() const;
  bool hasOnlyBrushes() const;
  bool hasPatches() const;
  bool hasOnlyPatches() const;
  bool hasBrushFaces() const;
  bool hasAnyBrushFaces() const;

  /**
   * For commands that modify entities, this returns all entities that should be acted on,
   * based on the current selection.
   *
   * - selected brushes/patches act on their parent entities
   * - selected groups implicitly act on any contained entities
   *
   * If multiple linked groups are selected, returns entities from all of them, so
   * attempting to perform commands on all of them will be blocked as a conflict.
   */
  const std::vector<EntityNodeBase*>& allEntities() const;

  /**
   * For commands that modify brushes, this returns all brushes that should be acted on,
   * based on the current selection.
   *
   * - selected groups implicitly act on any contained brushes
   *
   * If multiple linked groups are selected, returns brushes from all of them, so
   * attempting to perform commands on all of them will be blocked as a conflict.
   */
  const std::vector<BrushNode*>& allBrushes() const;

  const std::vector<BrushFaceHandle>& allBrushFaces() const;
};

Selection computeSelection(WorldNode& rootNode);

} // namespace tb::mdl
