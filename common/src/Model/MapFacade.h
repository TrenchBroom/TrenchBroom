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

#ifndef TrenchBroom_MapFacade
#define TrenchBroom_MapFacade

#include "TrenchBroom.h"
#include "Assets/AssetTypes.h"
#include "Model/EntityColor.h"
#include "Model/ModelTypes.h"

#include <vecmath/forward.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionManager;
        class TextureManager;
    }
    namespace Model {
        class BrushFaceAttributes;
        class ChangeBrushFaceAttributesRequest;
        class NodeCollection;

        class MapFacade {
        public:
            virtual ~MapFacade();
        public: // getters
            virtual Model::GameSPtr game() const = 0;

            virtual Layer* currentLayer() const = 0;
            virtual Group* currentGroup() const = 0;
            virtual Node* currentParent() const = 0;

            virtual Assets::EntityDefinitionManager& entityDefinitionManager() = 0;
            virtual Assets::EntityModelManager& entityModelManager() = 0;
            virtual Assets::TextureManager& textureManager() = 0;
        public: // selection
            virtual bool hasSelection() const = 0;
            virtual bool hasSelectedNodes() const = 0;
            virtual bool hasSelectedBrushFaces() const = 0;

            virtual const AttributableNodeList allSelectedAttributableNodes() const = 0;
            virtual const NodeCollection& selectedNodes() const = 0;
            virtual const BrushFaceList allSelectedBrushFaces() const = 0;
            virtual const BrushFaceList& selectedBrushFaces() const = 0;

            virtual const vm::bbox3& referenceBounds() const = 0;
            virtual const vm::bbox3& lastSelectionBounds() const = 0;
            virtual const vm::bbox3& selectionBounds() const = 0;
            virtual const String& currentTextureName() const = 0;

            virtual void selectAllNodes() = 0;
            virtual void selectSiblings() = 0;
            virtual void selectTouching(bool del) = 0;
            virtual void selectInside(bool del) = 0;
            virtual void selectNodesWithFilePosition(const std::vector<size_t>& positions) = 0;
            virtual void select(const NodeList& nodes) = 0;
            virtual void select(Node* node) = 0;
            virtual void select(const BrushFaceList& faces) = 0;
            virtual void select(BrushFace* face) = 0;
            virtual void convertToFaceSelection() = 0;

            virtual void deselectAll() = 0;
            virtual void deselect(Node* node) = 0;
            virtual void deselect(const NodeList& nodes) = 0;
            virtual void deselect(BrushFace* face) = 0;
        public: // adding, removing, reparenting, and duplicating nodes
            virtual void addNode(Node* node, Node* parent) = 0;
            virtual void removeNode(Node* node) = 0;

            virtual NodeList addNodes(const ParentChildrenMap& nodes) = 0;
            virtual NodeList addNodes(const NodeList& nodes, Node* parent) = 0;
            virtual void removeNodes(const NodeList& nodes) = 0;

            virtual bool reparentNodes(Node* newParent, const NodeList& children) = 0;
            virtual bool reparentNodes(const ParentChildrenMap& nodes) = 0;
            virtual bool deleteObjects() = 0;
            virtual bool duplicateObjects() = 0;
        public: // entity management
            virtual Model::Entity* createPointEntity(const Assets::PointEntityDefinition* definition, const vm::vec3& delta) = 0;
            virtual Model::Entity* createBrushEntity(const Assets::BrushEntityDefinition* definition) = 0;
        public: // modifying transient node attributes
            virtual void hide(const NodeList nodes) = 0; // Don't take the nodes by reference!
            virtual void show(const NodeList& nodes) = 0;
            virtual void resetVisibility(const NodeList& nodes) = 0;

            virtual void lock(const NodeList& nodes) = 0;
            virtual void unlock(const NodeList& nodes) = 0;
            virtual void resetLock(const NodeList& nodes) = 0;
        public: // modifying objects
            virtual bool translateObjects(const vm::vec3& delta) = 0;
            virtual bool rotateObjects(const vm::vec3& center, const vm::vec3& axis, FloatType angle) = 0;
            virtual bool scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) = 0;
            virtual bool scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) = 0;
            virtual bool shearObjects(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) = 0;
            virtual bool flipObjects(const vm::vec3& center, vm::axis::type axis) = 0;
        public: // modifying entity attributes
            virtual bool setAttribute(const AttributeName& name, const AttributeValue& value) = 0;
            virtual bool renameAttribute(const AttributeName& oldName, const AttributeName& newName) = 0;
            virtual bool removeAttribute(const AttributeName& name) = 0;

            virtual bool convertEntityColorRange(const AttributeName& name, Assets::ColorRange::Type range) = 0;
            virtual bool updateSpawnflag(const AttributeName& name, size_t flagIndex, bool setFlag) = 0;
        public: // brush resizing
            virtual bool resizeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) = 0;
        public: // modifying face attributes
            virtual void setTexture(Assets::Texture* texture) = 0;
            virtual bool setFaceAttributes(const BrushFaceAttributes& attributes) = 0;
            virtual bool setFaceAttributes(const ChangeBrushFaceAttributesRequest& request) = 0;
            virtual bool moveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) = 0;
            virtual bool rotateTextures(float angle) = 0;
            virtual bool shearTextures(const vm::vec2f& factors) = 0;
        public: // modifying vertices
            virtual void rebuildBrushGeometry(const BrushList& brushes) = 0;
            virtual bool snapVertices(FloatType snapTo) = 0;
            virtual bool findPlanePoints() = 0;

            struct MoveVerticesResult {
                bool success;
                bool hasRemainingVertices;
                MoveVerticesResult(bool i_success, bool i_hasRemainingVertices);
            };

            virtual MoveVerticesResult moveVertices(const VertexToBrushesMap& vertices, const vm::vec3& delta) = 0;
            virtual bool moveEdges(const EdgeToBrushesMap& edges, const vm::vec3& delta) = 0;
            virtual bool moveFaces(const FaceToBrushesMap& faces, const vm::vec3& delta) = 0;
        public: // search paths and mods
            virtual StringList mods() const = 0;
            virtual void setMods(const StringList& mods) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapFacade) */
