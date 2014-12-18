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

#ifndef __TrenchBroom__MapDocumentCommandFacade__
#define __TrenchBroom__MapDocumentCommandFacade__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/CollectUniqueNodesVisitor.h"
#include "Model/EntityAttributes.h"
#include "Model/Node.h"
#include "View/CommandProcessor.h"
#include "View/EntityColor.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Model {
        class ChangeBrushFaceAttributesRequest;
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManager;
        
        class MapDocumentCommandFacade : public MapDocument {
        private:
            CommandProcessor m_commandProcessor;
        public:
            static MapDocumentSPtr newMapDocument();
        private:
            MapDocumentCommandFacade();
        public: // selection modification
            void performSelect(const Model::NodeList& nodes);
            void performSelect(const Model::BrushFaceList& faces);
            void performSelectAllNodes();
            void performSelectAllBrushFaces();
            void performConvertToBrushFaceSelection();
            
            void performDeselect(const Model::NodeList& nodes);
            void performDeselect(const Model::BrushFaceList& faces);
            void performDeselectAll();
        private:
            void deselectAllNodes();
            void deselectAllBrushFaces();
        public: // adding and removing nodes
            Model::NodeList performAddNodes(const Model::ParentChildrenMap& nodes);
            Model::ParentChildrenMap performRemoveNodes(const Model::NodeList& nodes);
            Model::ParentChildrenMap performReparentNodes(const Model::ParentChildrenMap& nodes);
        public: // transformation
            void performTransform(const Mat4x4& transform, bool lockTextures);
        public: // entity attributes
            Model::EntityAttribute::Map performSetAttribute(const Model::AttributeName& name, const Model::AttributeValue& value);
            Model::EntityAttribute::Map performRemoveAttribute(const Model::AttributeName& name);
            Model::EntityAttribute::Map performConvertColorRange(const Model::AttributeName& name, ColorRange::Type colorRange);
            void performRenameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName);
            void restoreAttributes(const Model::EntityAttribute::Map& attributes);
        public: // brush face attributes
            void performMoveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);
            void performRotateTextures(float angle);
            void performShearTextures(const Vec2f& factors);
            void performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request);
        public: // vertices
            Vec3::List performSnapVertices(const Model::BrushVerticesMap& vertices, size_t snapTo);
            Vec3::List performMoveVertices(const Model::BrushVerticesMap& vertices, const Vec3& delta);
            Edge3::List performMoveEdges(const Model::BrushEdgesMap& edges, const Vec3& delta);
            Polygon3::List performMoveFaces(const Model::BrushFacesMap& faces, const Vec3& delta);
            Vec3::List performSplitEdges(const Model::BrushEdgesMap& edges, const Vec3& delta);
            Vec3::List performSplitFaces(const Model::BrushFacesMap& faces, const Vec3& delta);
        private: // implement MapDocument operations
            void performRebuildBrushGeometry(const Model::BrushList& brushes);
        public: // snapshots and restoration
            void restoreSnapshot(Model::Snapshot* snapshot);
        public: // entity definition file management
            void performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);
        public: // texture collection management
            void performAddExternalTextureCollections(const StringList& names);
            void performRemoveExternalTextureCollections(const StringList& names);
            void performMoveExternalTextureCollectionUp(const String& name);
            void performMoveExternalTextureCollectionDown(const String& name);
        public: // mods management
            void performSetMods(const StringList& mods);
        private: // helper methods
            template <typename I>
            Model::NodeList collectParents(const I begin, const I end) const {
                Model::CollectUniqueNodesVisitor visitor;
                Model::Node::escalate(begin, end, visitor);
                return visitor.nodes();
            }
            
            Model::NodeList collectParents(const Model::NodeList& nodes) const;
            Model::NodeList collectParents(const Model::ParentChildrenMap& nodes) const;
            Model::NodeList collectChildren(const Model::ParentChildrenMap& nodes) const;
            
            Model::ParentChildrenMap parentChildrenMap(const Model::NodeList& nodes) const;
            void addEmptyNodes(Model::ParentChildrenMap& nodes) const;
            Model::NodeList collectEmptyNodes(const Model::ParentChildrenMap& nodes) const;
            void removeEmptyNodes(Model::ParentChildrenMap& nodes, const Model::NodeList& emptyNodes) const;
        public: // modification count
            void incModificationCount(size_t delta = 1);
            void decModificationCount(size_t delta = 1);
        private: // implement MapDocument interface
            bool doCanUndoLastCommand() const;
            bool doCanRedoNextCommand() const;
            const String& doGetLastCommandName() const;
            const String& doGetNextCommandName() const;
            void doUndoLastCommand();
            void doRedoNextCommand();
            bool doRepeatLastCommands();
            void doClearRepeatableCommands();
            
            void doBeginTransaction(const String& name);
            void doEndTransaction();
            void doRollbackTransaction();

            bool doSubmit(UndoableCommand* command);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocumentCommandFacade__) */
