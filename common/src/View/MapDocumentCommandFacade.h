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

#ifndef TrenchBroom_MapDocumentCommandFacade
#define TrenchBroom_MapDocumentCommandFacade

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/EntityAttributeSnapshot.h"
#include "Model/EntityColor.h"
#include "Model/Node.h"
#include "View/CommandProcessor.h"
#include "View/MapDocument.h"
#include "View/UndoableCommand.h"

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
        private:
            void addEmptyNodes(Model::ParentChildrenMap& nodes) const;
            Model::NodeList collectEmptyNodes(const Model::ParentChildrenMap& nodes) const;
            void removeEmptyNodes(Model::ParentChildrenMap& nodes, const Model::NodeList& emptyNodes) const;
        public: // reparenting nodes
            struct ReparentResult {
                Model::ParentChildrenMap movedNodes;
                Model::ParentChildrenMap removedNodes;
                ReparentResult(const Model::ParentChildrenMap& i_movedNodes, const Model::ParentChildrenMap& i_removedNodes);
            };
            
            typedef enum {
                RemoveEmptyNodes,
                KeepEmptyNodes
            } EmptyNodePolicy;
            
            ReparentResult performReparentNodes(const Model::ParentChildrenMap& nodes, EmptyNodePolicy emptyNodePolicy);
        private:
            Model::NodeList findRemovableEmptyParentNodes(const Model::ParentChildrenMap& nodes) const;
        public: // Node Visibility
            Model::VisibilityMap setVisibilityState(const Model::NodeList& nodes, Model::VisibilityState visibilityState);
            Model::VisibilityMap setVisibilityEnsured(const Model::NodeList& nodes);
            void restoreVisibilityState(const Model::VisibilityMap& nodes);
            Model::LockStateMap setLockState(const Model::NodeList& nodes, Model::LockState lockState);
            void restoreLockState(const Model::LockStateMap& nodes);
        private:  // groups
            class RenameGroupsVisitor;
            class UndoRenameGroupsVisitor;
        public:
            Model::GroupNameMap performRenameGroups(const String& newName);
            void performUndoRenameGroups(const Model::GroupNameMap& newNames);
            
            void performPushGroup(Model::Group* group);
            void performPopGroup();
        public: // transformation
            void performTransform(const Mat4x4& transform, bool lockTextures);
        public: // entity attributes
            Model::EntityAttributeSnapshot::Map performSetAttribute(const Model::AttributeName& name, const Model::AttributeValue& value);
            Model::EntityAttributeSnapshot::Map performRemoveAttribute(const Model::AttributeName& name);
            Model::EntityAttributeSnapshot::Map performConvertColorRange(const Model::AttributeName& name, Assets::ColorRange::Type colorRange);
            void performRenameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName);
            void restoreAttributes(const Model::EntityAttributeSnapshot::Map& attributes);
        public: // brush resizing
            bool performResizeBrushes(const Model::BrushFaceList& faces, const Vec3& delta);
        public: // brush face attributes
            void performMoveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);
            void performRotateTextures(float angle);
            void performShearTextures(const Vec2f& factors);
            void performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request);
        public: // vertices
            Model::Snapshot* performFindPlanePoints();
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
        private:
            void doSetIssueHidden(Model::Issue* issue, bool hidden);
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

            bool doSubmit(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_MapDocumentCommandFacade) */
