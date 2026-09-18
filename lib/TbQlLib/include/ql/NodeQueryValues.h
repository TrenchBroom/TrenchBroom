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
} // namespace tb::mdl

namespace tb::ql
{

/**
 * Value-producing helpers for the fields that apply identically across several node
 * kinds' Binding (WorldNodeBinding, LayerNodeBinding, ...).
 */

el::Value layerNameValue(const mdl::Node& node);
el::Value groupNameValue(const mdl::Node& node);
bool isLinked(const mdl::Map& map, const mdl::Node& node);

/**
 * The names of every smart tag in `map` that `taggable` currently has -- the `tags`
 * field's value, for the node kinds (and BrushFace) that carry tags.
 */
el::Value tagsValue(const mdl::Map& map, const mdl::Taggable& taggable);

/**
 * `{classname, properties}` of `owner`'s entity, or Undefined if `owner` is null -- the
 * `entity` field's value for BrushNodeBinding and PatchNodeBinding, whose owning
 * entity is reached via BrushNode::entity()/PatchNode::entity() rather than held
 * directly.
 */
el::Value ownerEntityValue(const mdl::EntityNodeBase* owner);

} // namespace tb::ql
