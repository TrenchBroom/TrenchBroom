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

#include <string>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb::mdl
{
class BrushNode;
class BrushFace;
class EntityProperty;
class GroupNode;
class LayerNode;
class Node;
class PatchNode;
class WorldNode;

/**
 * Interface for stream-based serialization of a map, with public functions to
 * write different types of nodes to the output stream.
 *
 * The usage flow looks like:
 *
 * - construct a NodeSerializer
 * - call setExporting() to configure whether to write "omit from export" layers
 * - call beginFile() with all of the nodes that will be later serialized
 *   so subclasses can parallelize precomputing the serialization
 * - call e.g defaultLayer() to write that layer to the output
 * - call endFile()
 *
 * You may not reuse the NodeSerializer after that point.
 */
class NodeSerializer
{
protected:
  using ObjectNo = unsigned int;

private:
  ObjectNo m_entityNo = 0;
  ObjectNo m_brushNo = 0;
  bool m_exporting = false;
  bool m_stripTbProperties = false;

public:
  virtual ~NodeSerializer();

protected:
  ObjectNo entityNo() const;
  ObjectNo brushNo() const;

public:
  bool exporting() const;
  void setExporting(bool exporting);

  bool stripTbProperties() const;
  void setStripTbProperties(bool stripTbProperties);

public:
  /**
   * Prepares to serialize the given nodes and all of their children.
   * The order is ignored.
   *
   * The rootNodes parameter allows subclasses to optionally precompute the
   * serializations of all nodes in parallel.
   *
   * Any nodes serialized after calling beginFile() must have either been
   * in the rootNodes vector or be a descendant of one of these nodes.
   */
  void beginFile(
    const std::vector<const Node*>& rootNodes, kdl::task_manager& taskManager);
  void endFile();

public:
  void defaultLayer(const WorldNode& world);
  void customLayer(const LayerNode* layer);
  void group(const GroupNode* group, const std::vector<EntityProperty>& parentProperties);

  void entity(
    const Node* node,
    const std::vector<EntityProperty>& properties,
    const std::vector<EntityProperty>& parentProperties,
    const Node* brushParent);
  void entity(
    const Node* node,
    const std::vector<EntityProperty>& properties,
    const std::vector<EntityProperty>& parentProperties,
    const std::vector<BrushNode*>& entityBrushes);

private:
  void beginEntity(
    const Node* node,
    const std::vector<EntityProperty>& properties,
    const std::vector<EntityProperty>& extraAttributes);
  void beginEntity(const Node* node);
  void endEntity(const Node* node);

  void entityProperties(const std::vector<EntityProperty>& properties);
  void entityProperty(const EntityProperty& property);

  void brushes(const std::vector<BrushNode*>& brushNodes);
  void brush(const BrushNode* brushNode);

  void patch(const PatchNode* patchNode);

public:
  void brushFaces(const std::vector<BrushFace>& faces);

private:
  void brushFace(const BrushFace& face);

public:
  std::vector<EntityProperty> parentProperties(const Node* groupNode);

private:
  std::vector<EntityProperty> layerProperties(const LayerNode* layerNode);
  std::vector<EntityProperty> groupProperties(const GroupNode* groupNode);

protected:
  std::string escapeEntityProperties(const std::string& str) const;

private:
  virtual void doBeginFile(
    const std::vector<const Node*>& nodes, kdl::task_manager& taskManager) = 0;
  virtual void doEndFile() = 0;

  virtual void doBeginEntity(const Node* node) = 0;
  virtual void doEndEntity(const Node* node) = 0;
  virtual void doEntityProperty(const EntityProperty& property) = 0;

  virtual void doBrush(const BrushNode* brushNode) = 0;
  virtual void doBrushFace(const BrushFace& face) = 0;

  virtual void doPatch(const PatchNode* patchNode) = 0;
};

} // namespace tb::mdl
