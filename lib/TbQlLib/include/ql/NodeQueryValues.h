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

#include "el/LazyMap.h"
#include "el/Value.h"

namespace tb
{
namespace mdl
{
class EntityNodeBase;
class Map;
class Node;
class Taggable;
} // namespace mdl

namespace ql
{

el::Value layerNameValue(const mdl::Node& node);
el::Value groupNameValue(const mdl::Node& node);

el::Value tagsValue(const mdl::Map& map, const mdl::Taggable& taggable);

el::Value ownerEntityValue(const mdl::EntityNodeBase* owner);

/**
 * Adds the fields that layers, groups, entities, brushes and patches have in common to
 * the given fields of a lazy map that refers to its node as `node`.
 */
template <typename T>
el::LazyMapFields<T> withCommonNodeFields(el::LazyMapFields<T> fields)
{
  fields.insert({
    {"bounds",
     {el::ValueType::BBox,
      [](const auto& l) { return el::Value{l.node.logicalBounds()}; }}},
    {"center",
     {el::ValueType::Vec3,
      [](const auto& l) { return el::Value{l.node.logicalBounds().center()}; }}},
    {"visible",
     {el::ValueType::Boolean, [](const auto& l) { return el::Value{l.node.visible()}; }}},
    {"locked",
     {el::ValueType::Boolean, [](const auto& l) { return el::Value{l.node.locked()}; }}},
    {"layerName",
     {el::ValueType::String, [](const auto& l) { return layerNameValue(l.node); }}},
    {"groupName",
     {el::ValueType::String, [](const auto& l) { return groupNameValue(l.node); }}},
  });
  return fields;
}

} // namespace ql
} // namespace tb
