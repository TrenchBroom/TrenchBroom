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

namespace tb
{
namespace mdl
{
class BrushNode;
class BrushFace;
class EntityProperty;
class GroupNode;
class LayerNode;
class Node;
class PatchNode;
class WorldNode;
} // namespace mdl

namespace io
{
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

public:
  virtual ~NodeSerializer();

protected:
  ObjectNo entityNo() const;
  ObjectNo brushNo() const;

public:
  bool exporting() const;
  void setExporting(bool exporting);

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
    const std::vector<const mdl::Node*>& rootNodes, kdl::task_manager& taskManager);
  void endFile();

public:
  void defaultLayer(const mdl::WorldNode& world);
  void customLayer(const mdl::LayerNode* layer);
  void group(
    const mdl::GroupNode* group,
    const std::vector<mdl::EntityProperty>& parentProperties);

  void entity(
    const mdl::Node* node,
    const std::vector<mdl::EntityProperty>& properties,
    const std::vector<mdl::EntityProperty>& parentProperties,
    const mdl::Node* brushParent);
  void entity(
    const mdl::Node* node,
    const std::vector<mdl::EntityProperty>& properties,
    const std::vector<mdl::EntityProperty>& parentProperties,
    const std::vector<mdl::BrushNode*>& entityBrushes);

private:
  void beginEntity(
    const mdl::Node* node,
    const std::vector<mdl::EntityProperty>& properties,
    const std::vector<mdl::EntityProperty>& extraAttributes);
  void beginEntity(const mdl::Node* node);
  void endEntity(const mdl::Node* node);

  void entityProperties(const std::vector<mdl::EntityProperty>& properties);
  void entityProperty(const mdl::EntityProperty& property);

  void brushes(const std::vector<mdl::BrushNode*>& brushNodes);
  void brush(const mdl::BrushNode* brushNode);

  void patch(const mdl::PatchNode* patchNode);

public:
  void brushFaces(const std::vector<mdl::BrushFace>& faces);

private:
  void brushFace(const mdl::BrushFace& face);

public:
  std::vector<mdl::EntityProperty> parentProperties(const mdl::Node* groupNode);

private:
  std::vector<mdl::EntityProperty> layerProperties(const mdl::LayerNode* layerNode);
  std::vector<mdl::EntityProperty> groupProperties(const mdl::GroupNode* groupNode);

protected:
  std::string escapeEntityProperties(const std::string& str) const;

private:
  virtual void doBeginFile(
    const std::vector<const mdl::Node*>& nodes, kdl::task_manager& taskManager) = 0;
  virtual void doEndFile() = 0;

  virtual void doBeginEntity(const mdl::Node* node) = 0;
  virtual void doEndEntity(const mdl::Node* node) = 0;
  virtual void doEntityProperty(const mdl::EntityProperty& property) = 0;

  virtual void doBrush(const mdl::BrushNode* brushNode) = 0;
  virtual void doBrushFace(const mdl::BrushFace& face) = 0;

  virtual void doPatch(const mdl::PatchNode* patchNode) = 0;
};
} // namespace io
} // namespace tb
