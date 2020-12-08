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
#include "Model/BrushFaceHandle.h"
#include "Model/EntityColor.h"

#include <vecmath/forward.h>
#include <vecmath/util.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class BrushEntityDefinition;
        class EntityDefinitionManager;
        class EntityModelManager;
        class PointEntityDefinition;
        class Texture;
        class TextureManager;
    }

    namespace Model {
        class BrushFace;
        class BrushNode;
        class BrushFaceAttributes;
        class ChangeBrushFaceAttributesRequest;
        class EntityNode;
        class Game;
        class GroupNode;
        class LayerNode;
        class Node;
        class NodeCollection;

        /**
         * Interface of MapDocument that is exposed to the Model package.
         * Exists mostly so Issues (from the Model package) can perform quick fixes.
         */
        class MapFacade {
        public:
            virtual ~MapFacade();
        public: // getters
            virtual std::shared_ptr<Model::Game> game() const = 0;

            virtual LayerNode* currentLayer() const = 0;
            virtual GroupNode* currentGroup() const = 0;
            virtual Node* currentGroupOrWorld() const = 0;
            virtual Node* parentForNodes(const std::vector<Node*>& nodes) const = 0;

            virtual Assets::EntityDefinitionManager& entityDefinitionManager() = 0;
            virtual Assets::EntityModelManager& entityModelManager() = 0;
            virtual Assets::TextureManager& textureManager() = 0;
        public: // selection
            virtual bool hasSelection() const = 0;
            virtual bool hasSelectedNodes() const = 0;
            virtual bool hasSelectedBrushFaces() const = 0;
            virtual bool hasAnySelectedBrushFaces() const = 0;

            virtual std::vector<AttributableNode*> allSelectedAttributableNodes() const = 0;
            virtual const NodeCollection& selectedNodes() const = 0;
            virtual std::vector<BrushFaceHandle> allSelectedBrushFaces() const = 0;
            virtual std::vector<BrushFaceHandle> selectedBrushFaces() const = 0;

            virtual const vm::bbox3& referenceBounds() const = 0;
            virtual const vm::bbox3& lastSelectionBounds() const = 0;
            virtual const vm::bbox3& selectionBounds() const = 0;
            virtual const std::string& currentTextureName() const = 0;

            virtual void selectAllNodes() = 0;
            virtual void selectSiblings() = 0;
            virtual void selectTouching(bool del) = 0;
            virtual void selectInside(bool del) = 0;
            virtual void selectInverse() = 0;
            virtual void selectNodesWithFilePosition(const std::vector<size_t>& positions) = 0;
            virtual void select(const std::vector<Node*>& nodes) = 0;
            virtual void select(Node* node) = 0;
            virtual void select(const std::vector<BrushFaceHandle>& handles) = 0;
            virtual void select(const BrushFaceHandle& handle) = 0;
            virtual void convertToFaceSelection() = 0;

            virtual void deselectAll() = 0;
            virtual void deselect(Node* node) = 0;
            virtual void deselect(const std::vector<Node*>& nodes) = 0;
            virtual void deselect(const BrushFaceHandle& handle) = 0;
        public: // adding, removing, reparenting, and duplicating nodes
            virtual void addNode(Node* node, Node* parent) = 0;
            virtual void removeNode(Node* node) = 0;

            virtual std::vector<Node*> addNodes(const std::map<Node*, std::vector<Node*>>& nodes) = 0;
            virtual std::vector<Node*> addNodes(const std::vector<Node*>& nodes, Node* parent) = 0;
            virtual void removeNodes(const std::vector<Node*>& nodes) = 0;

            virtual bool reparentNodes(Node* newParent, const std::vector<Node*>& children) = 0;
            virtual bool reparentNodes(const std::map<Node*, std::vector<Node*>>& nodes) = 0;
            virtual bool deleteObjects() = 0;
            virtual bool duplicateObjects() = 0;
        public: // entity management
            virtual Model::EntityNode* createPointEntity(const Assets::PointEntityDefinition* definition, const vm::vec3& delta) = 0;
            virtual Model::EntityNode* createBrushEntity(const Assets::BrushEntityDefinition* definition) = 0;
        public: // modifying transient node attributes
            virtual void hide(std::vector<Node*> nodes) = 0; // Don't take the nodes by reference!
            virtual void show(const std::vector<Node*>& nodes) = 0;
            virtual void resetVisibility(const std::vector<Node*>& nodes) = 0;

            virtual void lock(const std::vector<Node*>& nodes) = 0;
            virtual void unlock(const std::vector<Node*>& nodes) = 0;
            virtual void resetLock(const std::vector<Node*>& nodes) = 0;
        public: // modifying objects
            virtual bool translateObjects(const vm::vec3& delta) = 0;
            virtual bool rotateObjects(const vm::vec3& center, const vm::vec3& axis, FloatType angle) = 0;
            virtual bool scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) = 0;
            virtual bool scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) = 0;
            virtual bool shearObjects(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) = 0;
            virtual bool flipObjects(const vm::vec3& center, vm::axis::type axis) = 0;
        public: // modifying entity attributes
            virtual bool setAttribute(const std::string& name, const std::string& value) = 0;
            virtual bool renameAttribute(const std::string& oldName, const std::string& newName) = 0;
            virtual bool removeAttribute(const std::string& name) = 0;

            virtual bool convertEntityColorRange(const std::string& name, Assets::ColorRange::Type range) = 0;
            virtual bool updateSpawnflag(const std::string& name, size_t flagIndex, bool setFlag) = 0;
        public: // brush resizing
            virtual bool resizeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) = 0;
        public: // modifying face attributes
            virtual bool setFaceAttributes(const BrushFaceAttributes& attributes) = 0;
            virtual bool setFaceAttributesExceptContentFlags(const BrushFaceAttributes& attributes) = 0;
            virtual bool setFaceAttributes(const ChangeBrushFaceAttributesRequest& request) = 0;
            virtual bool moveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) = 0;
            virtual bool rotateTextures(float angle) = 0;
            virtual bool shearTextures(const vm::vec2f& factors) = 0;
        public: // modifying vertices
            virtual bool snapVertices(FloatType snapTo) = 0;

            struct MoveVerticesResult {
                bool success;
                bool hasRemainingVertices;
                MoveVerticesResult(bool i_success, bool i_hasRemainingVertices);
            };

            virtual MoveVerticesResult moveVertices(std::vector<vm::vec3> vertexPositions, const vm::vec3& delta) = 0;
            virtual MoveVerticesResult moveVertices(const std::map<vm::vec3, std::vector<BrushNode*>>& vertices, const vm::vec3& delta) = 0;
            virtual bool moveEdges(const std::map<vm::segment3, std::vector<BrushNode*>>& edges, const vm::vec3& delta) = 0;
            virtual bool moveFaces(const std::map<vm::polygon3, std::vector<BrushNode*>>& faces, const vm::vec3& delta) = 0;
        public: // search paths and mods
            virtual std::vector<std::string> mods() const = 0;
            virtual void setMods(const std::vector<std::string>& mods) = 0;
        };
    }
}

