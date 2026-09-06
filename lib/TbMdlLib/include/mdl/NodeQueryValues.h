/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"

namespace tb::mdl
{
class EntityNodeBase;
class Map;
class Node;
class Taggable;

/**
 * Value-producing helpers for the fields that apply identically across several node
 * kinds' BoundValue (WorldNodeBoundValue, LayerNodeBoundValue, ...).
 */

el::Value layerNameValue(const Node& node);
el::Value groupNameValue(const Node& node);
bool isLinked(const Map& map, const Node& node);

/**
 * The names of every smart tag in `map` that `taggable` currently has -- the `tags`
 * field's value, for the node kinds (and BrushFace) that carry tags.
 */
el::Value tagsValue(const Map& map, const Taggable& taggable);

/**
 * `{classname, properties}` of `owner`'s entity, or Undefined if `owner` is null -- the
 * `entity` field's value for BrushNodeBoundValue and PatchNodeBoundValue, whose owning
 * entity is reached via BrushNode::entity()/PatchNode::entity() rather than held
 * directly.
 */
el::Value ownerEntityValue(const EntityNodeBase* owner);

} // namespace tb::mdl
