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

#include "FloatType.h"
#include "Macros.h"
#include "Model/IdType.h"
#include "Model/Layer.h"
#include "Model/Node.h"

#include <vecmath/bbox.h>

#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class LayerNode : public Node
{
private:
  Layer m_layer;

  mutable vm::bbox3 m_logicalBounds;
  mutable vm::bbox3 m_physicalBounds;
  mutable bool m_boundsValid;

  /**
   * The ID used to serialize layer nodes (see MapReader and NodeSerializer). This is set
   * by MapReader when a layer is read, or by WorldNode when a layer is added that doesn't
   * yet have a persistent ID.
   */
  std::optional<IdType> m_persistentId;

public:
  explicit LayerNode(Layer layer);

  const Layer& layer() const;
  Layer setLayer(Layer layer);

  bool isDefaultLayer() const;

  /**
   * Stable sort the given vector using `sortIndex()` as the sort key.
   */
  static void sortLayers(std::vector<LayerNode*>& layers);

  const std::optional<IdType>& persistentId() const;
  void setPersistentId(IdType persistentId);

private: // implement Node interface
  const std::string& doGetName() const override;
  const vm::bbox3& doGetLogicalBounds() const override;
  const vm::bbox3& doGetPhysicalBounds() const override;
  FloatType doGetProjectedArea(vm::axis::type axis) const override;

  Node* doClone(const vm::bbox3& worldBounds, SetLinkId setLinkIds) const override;
  bool doCanAddChild(const Node* child) const override;
  bool doCanRemoveChild(const Node* child) const override;
  bool doRemoveIfEmpty() const override;
  bool doShouldAddToSpacialIndex() const override;
  void doNodePhysicalBoundsDidChange() override;
  bool doSelectable() const override;

  void doPick(
    const EditorContext& editorContext,
    const vm::ray3& ray,
    PickResult& pickResult) override;
  void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

  void doAccept(NodeVisitor& visitor) override;
  void doAccept(ConstNodeVisitor& visitor) const override;

private:
  void invalidateBounds();
  void validateBounds() const;

private: // implement Taggable interface
  void doAcceptTagVisitor(TagVisitor& visitor) override;
  void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;

private:
  deleteCopyAndMove(LayerNode);
};
} // namespace Model
} // namespace TrenchBroom
