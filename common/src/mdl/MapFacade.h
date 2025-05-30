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

#include "mdl/BrushFaceHandle.h"
#include "mdl/EntityColor.h"

#include "vm/bbox.h"
#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tb::mdl
{
class BrushFace;
class BrushFaceAttributes;
class BrushNode;
class ChangeBrushFaceAttributesRequest;
struct EntityDefinition;
class EntityDefinitionManager;
class EntityModelManager;
class EntityNode;
class Game;
class GroupNode;
class LayerNode;
class Material;
class MaterialManager;
class Node;
struct Selection;

/**
 * Interface of MapDocument that is exposed to the Model package.
 * Exists mostly so Issues (from the Model package) can perform quick fixes.
 */
class MapFacade
{
public:
  virtual ~MapFacade();

public: // getters
  virtual std::shared_ptr<mdl::Game> game() const = 0;

  virtual LayerNode* currentLayer() const = 0;
  virtual GroupNode* currentGroup() const = 0;
  virtual Node* currentGroupOrWorld() const = 0;
  virtual Node* parentForNodes(const std::vector<Node*>& nodes) const = 0;

  virtual EntityDefinitionManager& entityDefinitionManager() = 0;
  virtual EntityModelManager& entityModelManager() = 0;
  virtual MaterialManager& materialManager() = 0;

public: // selection
  virtual bool hasSelection() const = 0;
  virtual bool hasSelectedNodes() const = 0;
  virtual bool hasSelectedBrushFaces() const = 0;
  virtual bool hasAnySelectedBrushFaces() const = 0;

  virtual std::vector<EntityNodeBase*> allSelectedEntityNodes() const = 0;
  virtual const Selection& selection() const = 0;
  virtual std::vector<BrushFaceHandle> allSelectedBrushFaces() const = 0;
  virtual std::vector<BrushFaceHandle> selectedBrushFaces() const = 0;

  virtual const vm::bbox3d referenceBounds() const = 0;
  virtual const std::optional<vm::bbox3d>& lastSelectionBounds() const = 0;
  virtual const std::optional<vm::bbox3d>& selectionBounds() const = 0;
  virtual const std::string& currentMaterialName() const = 0;

  virtual void selectAllNodes() = 0;
  virtual void selectSiblings() = 0;
  virtual void selectTouching(bool del) = 0;
  virtual void selectInside(bool del) = 0;
  virtual void selectInverse() = 0;
  virtual void selectNodesWithFilePosition(const std::vector<size_t>& positions) = 0;
  virtual void selectNodes(const std::vector<Node*>& nodes) = 0;
  virtual void selectBrushFaces(const std::vector<BrushFaceHandle>& handles) = 0;
  virtual void convertToFaceSelection() = 0;

  virtual void deselectAll() = 0;
  virtual void deselectNodes(const std::vector<Node*>& nodes) = 0;
  virtual void deselectBrushFaces(const std::vector<BrushFaceHandle>& handles) = 0;

public: // adding, removing, reparenting, and duplicating nodes
  virtual std::vector<Node*> addNodes(
    const std::map<Node*, std::vector<Node*>>& nodes) = 0;
  virtual void removeNodes(const std::vector<Node*>& nodes) = 0;

  virtual bool reparentNodes(const std::map<Node*, std::vector<Node*>>& nodes) = 0;
  virtual void remove() = 0;
  virtual void duplicate() = 0;

public: // entity management
  virtual mdl::EntityNode* createPointEntity(
    const EntityDefinition& definition, const vm::vec3d& delta) = 0;
  virtual mdl::EntityNode* createBrushEntity(const EntityDefinition& definition) = 0;

public:                                            // modifying transient node attributes
  virtual void hide(std::vector<Node*> nodes) = 0; // Don't take the nodes by reference!
  virtual void show(const std::vector<Node*>& nodes) = 0;
  virtual void resetVisibility(const std::vector<Node*>& nodes) = 0;

  virtual void lock(const std::vector<Node*>& nodes) = 0;
  virtual void unlock(const std::vector<Node*>& nodes) = 0;
  virtual void resetLock(const std::vector<Node*>& nodes) = 0;

public: // modifying objects
  virtual bool translate(const vm::vec3d& delta) = 0;
  virtual bool rotate(const vm::vec3d& center, const vm::vec3d& axis, double angle) = 0;
  virtual bool scale(const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox) = 0;
  virtual bool scale(const vm::vec3d& center, const vm::vec3d& scaleFactors) = 0;
  virtual bool shear(
    const vm::bbox3d& box, const vm::vec3d& sideToShear, const vm::vec3d& delta) = 0;
  virtual bool flip(const vm::vec3d& center, vm::axis::type axis) = 0;

public: // modifying entity properties
  virtual bool setProperty(
    const std::string& key,
    const std::string& value,
    bool defaultToProtected = false) = 0;
  virtual bool renameProperty(const std::string& oldKey, const std::string& newKey) = 0;
  virtual bool removeProperty(const std::string& key) = 0;

  virtual bool convertEntityColorRange(
    const std::string& name, ColorRange::Type range) = 0;
  virtual bool updateSpawnflag(
    const std::string& name, size_t flagIndex, bool setFlag) = 0;

public: // brush extrusion
  virtual bool extrudeBrushes(
    const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta) = 0;

public: // modifying face attributes
  virtual bool setFaceAttributes(const BrushFaceAttributes& attributes) = 0;
  virtual bool setFaceAttributesExceptContentFlags(
    const BrushFaceAttributes& attributes) = 0;
  virtual bool setFaceAttributes(const ChangeBrushFaceAttributesRequest& request) = 0;
  virtual bool translateUV(
    const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) = 0;
  virtual bool rotateUV(float angle) = 0;
  virtual bool shearUV(const vm::vec2f& factors) = 0;

public: // modifying vertices
  virtual bool snapVertices(double snapTo) = 0;

  struct TransformVerticesResult
  {
    bool success;
    bool hasRemainingVertices;
  };

  virtual TransformVerticesResult transformVertices(
    std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform) = 0;
  virtual bool transformEdges(
    std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform) = 0;
  virtual bool transformFaces(
    std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform) = 0;

public: // search paths and mods
  virtual std::vector<std::string> mods() const = 0;
  virtual void setMods(const std::vector<std::string>& mods) = 0;
};

} // namespace tb::mdl
