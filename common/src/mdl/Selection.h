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
class GroupNode;
class LayerNode;
class Node;
class PatchNode;

struct Selection
{
  std::vector<Node*> nodes;
  std::vector<GroupNode*> groups;
  std::vector<EntityNode*> entities;
  std::vector<BrushNode*> brushes;
  std::vector<PatchNode*> patches;
  std::vector<mdl::BrushFaceHandle> brushFaces;

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

  void addNodes(const std::vector<Node*>& nodes);
  void addNode(Node* node);

  void removeNodes(const std::vector<Node*>& nodes);
  void removeNode(Node* node);

  void clear();
};

Selection makeSelection(const std::vector<Node*>& nodes);

} // namespace tb::mdl
