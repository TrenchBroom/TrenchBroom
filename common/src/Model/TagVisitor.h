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

namespace TrenchBroom
{
namespace Model
{
class BrushNode;
class BrushFace;
class EntityNode;
class GroupNode;
class LayerNode;
class PatchNode;
class WorldNode;

class TagVisitor
{
public:
  virtual ~TagVisitor();

  virtual void visit(WorldNode& world);
  virtual void visit(LayerNode& layer);
  virtual void visit(GroupNode& group);
  virtual void visit(EntityNode& entity);
  virtual void visit(BrushNode& brush);
  virtual void visit(BrushFace& face);
  virtual void visit(PatchNode& patch);
};

class ConstTagVisitor
{
public:
  virtual ~ConstTagVisitor();

  virtual void visit(const WorldNode& world);
  virtual void visit(const LayerNode& layer);
  virtual void visit(const GroupNode& group);
  virtual void visit(const EntityNode& entity);
  virtual void visit(const BrushNode& brush);
  virtual void visit(const BrushFace& face);
  virtual void visit(const PatchNode& patch);
};
} // namespace Model
} // namespace TrenchBroom
