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

#include "Macros.h"
#include "mdl/Brush.h"
#include "mdl/BrushGeometry.h"
#include "mdl/HitType.h"
#include "mdl/Node.h"
#include "mdl/Object.h"
#include "mdl/TagType.h"

#include "vm/ray.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb::render
{
class BrushRendererBrushCache;
}

namespace tb::mdl
{
class BrushFace;
class GroupNode;
class LayerNode;
class Material;
class ModelFactory;

class BrushNode : public Node, public Object
{
public:
  static const HitType::Type BrushHitType;

public:
  using VertexList = BrushVertexList;
  using EdgeList = BrushEdgeList;

private:
  mutable std::unique_ptr<render::BrushRendererBrushCache>
    m_brushRendererBrushCache; // unique_ptr for breaking header dependencies
  Brush m_brush;               // must be destroyed before the brush renderer cache
  size_t m_selectedFaceCount = 0u;

public:
  explicit BrushNode(Brush brush);
  ~BrushNode() override;

public:
  EntityNodeBase* entity();
  const EntityNodeBase* entity() const;

  const Brush& brush() const;
  Brush setBrush(Brush brush);

  bool hasSelectedFaces() const;
  void selectFace(size_t faceIndex);
  void deselectFace(size_t faceIndex);

  void updateFaceTags(size_t faceIndex, TagManager& tagManager);

  void setFaceMaterial(size_t faceIndex, Material* material);

  bool contains(const Node* node) const;
  bool intersects(const Node* node) const;

private:
  void clearSelectedFaces();
  void updateSelectedFaceCount();

private: // implement Node interface
  const std::string& doGetName() const override;
  const vm::bbox3d& doGetLogicalBounds() const override;
  const vm::bbox3d& doGetPhysicalBounds() const override;

  double doGetProjectedArea(vm::axis::type axis) const override;

  Node* doClone(const vm::bbox3d& worldBounds) const override;

  bool doCanAddChild(const Node* child) const override;
  bool doCanRemoveChild(const Node* child) const override;
  bool doRemoveIfEmpty() const override;

  bool doShouldAddToSpacialIndex() const override;

  bool doSelectable() const override;

  void doAccept(NodeVisitor& visitor) override;
  void doAccept(ConstNodeVisitor& visitor) const override;

private: // implement Object interface
  void doPick(
    const EditorContext& editorContext,
    const vm::ray3d& ray,
    PickResult& pickResult) override;
  void doFindNodesContaining(const vm::vec3d& point, std::vector<Node*>& result) override;

  std::optional<std::tuple<double, size_t>> findFaceHit(const vm::ray3d& ray) const;

  Node* doGetContainer() override;
  LayerNode* doGetContainingLayer() override;
  GroupNode* doGetContainingGroup() override;

public: // renderer cache
  /**
   * Only exposed to be called by BrushFace
   */
  void invalidateVertexCache();
  render::BrushRendererBrushCache& brushRendererBrushCache() const;

private: // implement Taggable interface
public:
  void initializeTags(TagManager& tagManager) override;
  void clearTags() override;
  void updateTags(TagManager& tagManager) override;

  /**
   * Indicates whether all of the faces of this brush have any of the given tags.
   *
   * @param tagMask the tags to check
   * @return true whether all faces of this brush have any of the given tags
   */
  bool allFacesHaveAnyTagInMask(TagType::Type tagMask) const;

  /**
   * Indicates whether any of the faces of this brush have any tags.
   *
   * @return true whether any faces of this brush have any tags
   */
  bool anyFaceHasAnyTag() const;

  /**
   * Indicates whether any of the faces of this brush have any of the given tags.
   *
   * @param tagMask the tags to check
   * @return true whether any faces of this brush have any of the given tags
   */
  bool anyFacesHaveAnyTagInMask(TagType::Type tagMask) const;

private:
  void doAcceptTagVisitor(TagVisitor& visitor) override;
  void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;

private:
  deleteCopyAndMove(BrushNode);
};

bool operator==(const BrushNode& lhs, const BrushNode& rhs);
bool operator!=(const BrushNode& lhs, const BrushNode& rhs);

} // namespace tb::mdl
