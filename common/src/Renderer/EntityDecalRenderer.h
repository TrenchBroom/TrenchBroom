/*
 Copyright (C) 2023 Daniel Walder

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

#include "Color.h"
#include "Renderer/AllocationTracker.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"
#include "Renderer/Renderable.h"

#include <kdl/vector_set.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace TrenchBroom::Assets
{
class Texture;
struct DecalSpecification;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
class BrushFace;
class BrushNode;
class EntityNode;
class Node;
} // namespace TrenchBroom::Model

namespace TrenchBroom::View
{
class MapDocument; // FIXME: Renderer should not depend on View
} // namespace TrenchBroom::View

namespace TrenchBroom::Renderer
{
class EntityDecalRenderer
{
private:
  struct EntityDecalData
  {
    std::vector<const Model::BrushNode*> brushes;

    /* will only be true if the brushes array has been calculated since the last change
     * and the decal geometry is stored in the VBO */
    bool validated = false;

    Assets::Texture* texture = nullptr;

    AllocationTracker::Block* vertexHolderKey = nullptr;
    AllocationTracker::Block* faceIndicesKey = nullptr;
  };

  using EntityWithDependenciesMap =
    std::unordered_map<const Model::EntityNode*, EntityDecalData>;

  std::weak_ptr<View::MapDocument> m_document;
  EntityWithDependenciesMap m_entities;

  using Vertex = Renderer::GLVertexTypes::P3NT2::Vertex;
  using TextureToBrushIndicesMap =
    std::unordered_map<const Assets::Texture*, std::shared_ptr<BrushIndexArray>>;

  std::shared_ptr<TextureToBrushIndicesMap> m_faces;
  std::shared_ptr<BrushVertexArray> m_vertexArray;
  FaceRenderer m_faceRenderer;
  Color m_faceColor;

public:
  explicit EntityDecalRenderer(std::weak_ptr<View::MapDocument> document);

  /**
   * Equivalent to updateNode() on all added nodes.
   */
  void invalidate();

  /**
   * Equivalent to removeNode() on all added nodes.
   */
  void clear();

  /**
   * Adds a node if its not already present and invalidates it.
   */
  void updateNode(Model::Node* node);

  /**
   * Removes a node. Calling with an unknown node is allowed, but ignored.
   */
  void removeNode(Model::Node* node);

private:
  void updateEntity(const Model::EntityNode* entityNode);
  void removeEntity(const Model::EntityNode* entityNode);
  void updateBrush(const Model::BrushNode* brushNode);
  void removeBrush(const Model::BrushNode* brushNode);

  void invalidateDecalData(EntityDecalData& data) const;

  void validateDecalData(
    const Model::EntityNode* entityNode, EntityDecalData& data) const;

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

  deleteCopy(EntityDecalRenderer);
};
} // namespace TrenchBroom::Renderer
