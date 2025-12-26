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

#include <map>
#include <memory>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
namespace mdl
{
class BrushNode;
class BrushFace;
class EntityNode;
class LayerNode;
class Node;
class NodeSerializer;
class WorldNode;

class NodeWriter
{
private:
  using EntityBrushesMap = std::map<EntityNode*, std::vector<BrushNode*>>;

  const WorldNode& m_world;
  std::unique_ptr<NodeSerializer> m_serializer;

public:
  NodeWriter(const WorldNode& world, std::ostream& stream);
  NodeWriter(const WorldNode& world, std::unique_ptr<NodeSerializer> serializer);
  ~NodeWriter();

  void setExporting(bool exporting);
  void writeMap(kdl::task_manager& taskManager);

private:
  void writeDefaultLayer();
  void writeCustomLayers();
  void writeCustomLayer(const LayerNode* layer);

public:
  void writeNodes(const std::vector<Node*>& nodes, kdl::task_manager& taskManager);

private:
  void writeWorldBrushes(const std::vector<BrushNode*>& brushes);
  void writeEntityBrushes(const EntityBrushesMap& entityBrushes);

public:
  void writeBrushFaces(
    const std::vector<BrushFace>& faces, kdl::task_manager& taskManager);
};

} // namespace mdl
} // namespace tb
