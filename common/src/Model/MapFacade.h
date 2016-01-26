/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Assets/AssetTypes.h"
#include "Model/EntityColor.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        class ChangeBrushFaceAttributesRequest;
        class NodeCollection;

        class MapFacade {
        public:
            virtual ~MapFacade();
        public: // selection
            virtual bool hasSelection() const = 0;
            virtual bool hasSelectedNodes() const = 0;
            virtual bool hasSelectedBrushFaces() const = 0;
            
            virtual const AttributableNodeList allSelectedAttributableNodes() const = 0;
            virtual const NodeCollection& selectedNodes() const = 0;
            virtual const BrushFaceList allSelectedBrushFaces() const = 0;
            virtual const BrushFaceList& selectedBrushFaces() const = 0;
            
            virtual const BBox3& referenceBounds() const = 0;
            virtual const BBox3& lastSelectionBounds() const = 0;
            virtual const BBox3& selectionBounds() const = 0;
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
            virtual void removeNodes(const NodeList& nodes) = 0;
            
            virtual void reparentNodes(Node* newParent, const NodeList& children) = 0;
            virtual void reparentNodes(const ParentChildrenMap& nodes) = 0;
            virtual bool deleteObjects() = 0;
            virtual bool duplicateObjects() = 0;
        public: // modifying transient node attributes
            virtual void hide(const Model::NodeList nodes) = 0;
            virtual void show(const Model::NodeList& nodes) = 0;
            virtual void resetVisibility(const Model::NodeList& nodes) = 0;
            
            virtual void lock(const Model::NodeList& nodes) = 0;
            virtual void unlock(const Model::NodeList& nodes) = 0;
            virtual void resetLock(const Model::NodeList& nodes) = 0;
        public: // modifying objects
            virtual bool translateObjects(const Vec3& delta) = 0;
            virtual bool rotateObjects(const Vec3& center, const Vec3& axis, FloatType angle) = 0;
            virtual bool flipObjects(const Vec3& center, Math::Axis::Type axis) = 0;
        public: // modifying entity attributes
            virtual bool setAttribute(const AttributeName& name, const AttributeValue& value) = 0;
            virtual bool renameAttribute(const AttributeName& oldName, const AttributeName& newName) = 0;
            virtual bool removeAttribute(const AttributeName& name) = 0;
            
            virtual bool convertEntityColorRange(const AttributeName& name, Assets::ColorRange::Type range) = 0;
        public: // brush resizing
            virtual bool resizeBrushes(const BrushFaceList& faces, const Vec3& delta) = 0;
        public: // modifying face attributes
            virtual bool setTexture(Assets::Texture* texture) = 0;
            virtual bool setFaceAttributes(const BrushFaceAttributes& attributes) = 0;
            virtual bool setFaceAttributes(const ChangeBrushFaceAttributesRequest& request) = 0;
            virtual bool moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) = 0;
            virtual bool rotateTextures(float angle) = 0;
            virtual bool shearTextures(const Vec2f& factors) = 0;
        public: // modifying vertices
            virtual void rebuildBrushGeometry(const BrushList& brushes) = 0;
            bool snapVertices();
            virtual bool snapVertices(const VertexToBrushesMap& vertices, size_t snapTo) = 0;
            virtual bool findPlanePoints() = 0;
            
            struct MoveVerticesResult {
                bool success;
                bool hasRemainingVertices;
                MoveVerticesResult(bool i_success, bool i_hasRemainingVertices);
            };
            
            virtual MoveVerticesResult moveVertices(const VertexToBrushesMap& vertices, const Vec3& delta) = 0;
            virtual bool moveEdges(const VertexToEdgesMap& edges, const Vec3& delta) = 0;
            virtual bool moveFaces(const VertexToFacesMap& faces, const Vec3& delta) = 0;
            virtual bool splitEdges(const VertexToEdgesMap& edges, const Vec3& delta) = 0;
            virtual bool splitFaces(const VertexToFacesMap& faces, const Vec3& delta) = 0;
        };
        
        class PushSelection {
        private:
            MapFacade* m_facade;
            NodeList m_nodes;
            BrushFaceList m_faces;
        public:
            PushSelection(MapFacade* facade);
            ~PushSelection();
        };
    }
}

#endif /* defined(TrenchBroom_MapFacade) */
