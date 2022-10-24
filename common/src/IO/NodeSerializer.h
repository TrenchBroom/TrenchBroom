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

#include "Model/IdType.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class BrushNode;
class BrushFace;
class EntityProperty;
class GroupNode;
class LayerNode;
class Node;
class PatchNode;
class WorldNode;
} // namespace Model

namespace IO
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
  ObjectNo m_entityNo;
  ObjectNo m_brushNo;

  bool m_exporting;

public:
  NodeSerializer();
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
  void beginFile(const std::vector<const Model::Node*>& rootNodes);
  void endFile();

public:
  void defaultLayer(const Model::WorldNode& world);
  void customLayer(const Model::LayerNode* layer);
  void group(
    const Model::GroupNode* group,
    const std::vector<Model::EntityProperty>& parentProperties);

  void entity(
    const Model::Node* node,
    const std::vector<Model::EntityProperty>& properties,
    const std::vector<Model::EntityProperty>& parentProperties,
    const Model::Node* brushParent);
  void entity(
    const Model::Node* node,
    const std::vector<Model::EntityProperty>& properties,
    const std::vector<Model::EntityProperty>& parentProperties,
    const std::vector<Model::BrushNode*>& entityBrushes);

private:
  void beginEntity(
    const Model::Node* node,
    const std::vector<Model::EntityProperty>& properties,
    const std::vector<Model::EntityProperty>& extraAttributes);
  void beginEntity(const Model::Node* node);
  void endEntity(const Model::Node* node);

  void entityProperties(const std::vector<Model::EntityProperty>& properties);
  void entityProperty(const Model::EntityProperty& property);

  void brushes(const std::vector<Model::BrushNode*>& brushNodes);
  void brush(const Model::BrushNode* brushNode);

  void patch(const Model::PatchNode* patchNode);

public:
  void brushFaces(const std::vector<Model::BrushFace>& faces);

private:
  void brushFace(const Model::BrushFace& face);

public:
  std::vector<Model::EntityProperty> parentProperties(const Model::Node* groupNode);

private:
  std::vector<Model::EntityProperty> layerProperties(const Model::LayerNode* layerNode);
  std::vector<Model::EntityProperty> groupProperties(const Model::GroupNode* groupNode);

protected:
  std::string escapeEntityProperties(const std::string& str) const;

private:
  virtual void doBeginFile(const std::vector<const Model::Node*>& nodes) = 0;
  virtual void doEndFile() = 0;

  virtual void doBeginEntity(const Model::Node* node) = 0;
  virtual void doEndEntity(const Model::Node* node) = 0;
  virtual void doEntityProperty(const Model::EntityProperty& property) = 0;

  virtual void doBrush(const Model::BrushNode* brushNode) = 0;
  virtual void doBrushFace(const Model::BrushFace& face) = 0;

  virtual void doPatch(const Model::PatchNode* patchNode) = 0;
};
} // namespace IO
} // namespace TrenchBroom
