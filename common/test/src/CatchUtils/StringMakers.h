/*
 Copyright (C) 2023 Kristian Duske

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

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include "Catch2.h"

namespace TrenchBroom::Model
{
class Node;

std::string convertToString(const Node& node);
} // namespace TrenchBroom::Model

namespace Catch
{
template <>
struct StringMaker<TrenchBroom::Model::Node>
{
  static std::string convert(const TrenchBroom::Model::Node& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::WorldNode>
{
  static std::string convert(const TrenchBroom::Model::WorldNode& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::LayerNode>
{
  static std::string convert(const TrenchBroom::Model::LayerNode& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::GroupNode>
{
  static std::string convert(const TrenchBroom::Model::GroupNode& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::EntityNode>
{
  static std::string convert(const TrenchBroom::Model::EntityNode& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::BrushNode>
{
  static std::string convert(const TrenchBroom::Model::BrushNode& value)
  {
    return convertToString(value);
  }
};

template <>
struct StringMaker<TrenchBroom::Model::PatchNode>
{
  static std::string convert(const TrenchBroom::Model::PatchNode& value)
  {
    return convertToString(value);
  }
};
} // namespace Catch