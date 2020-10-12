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

#ifndef TrenchBroom_MapDocumentCommandFacade
#define TrenchBroom_MapDocumentCommandFacade

#include "FloatType.h"
#include "View/MapDocument.h"

#include <vecmath/forward.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityAttributeSnapshot;
        enum class LockState;
        class Snapshot;
        enum class VisibilityState;
    }

    namespace View {
        class CommandProcessor;

        /**
         * MapDocument API that is private to Command classes.
         *
         * These `performSomething()` methods will actually do an action, where
         * the corresponding `something()` in MapDocument would create and execute a
         * Command object which then calls `performSomething()`.
         */
        class MapDocumentCommandFacade : public MapDocument {
        private:
            std::unique_ptr<CommandProcessor> m_commandProcessor;
        public:
            static std::shared_ptr<MapDocument> newMapDocument();
        private:
            MapDocumentCommandFacade();
        public:
            ~MapDocumentCommandFacade() override;
        public: // selection modification
            void performSelect(const std::vector<Model::Node*>& nodes);
            void performSelect(const std::vector<Model::BrushFaceHandle>& faces);
            void performSelectAllNodes();
            void performSelectAllBrushFaces();
            void performConvertToBrushFaceSelection();

            void performDeselect(const std::vector<Model::Node*>& nodes);
            void performDeselect(const std::vector<Model::BrushFaceHandle>& faces);
            void performDeselectAll();
        private:
            void deselectAllNodes();
            void deselectAllBrushFaces();
        public: // adding and removing nodes
            void performAddNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes);
            void performRemoveNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes);
        public: // Node Visibility
            std::map<Model::Node*, Model::VisibilityState> setVisibilityState(const std::vector<Model::Node*>& nodes, Model::VisibilityState visibilityState);
            std::map<Model::Node*, Model::VisibilityState> setVisibilityEnsured(const std::vector<Model::Node*>& nodes);
            void restoreVisibilityState(const std::map<Model::Node*, Model::VisibilityState>& nodes);
            std::map<Model::Node*, Model::LockState> setLockState(const std::vector<Model::Node*>& nodes, Model::LockState lockState);
            void restoreLockState(const std::map<Model::Node*, Model::LockState>& nodes);
        public: // layers
            using MapDocument::performSetCurrentLayer;
        private:  // groups
            class RenameGroupsVisitor;
            class UndoRenameGroupsVisitor;
        public:
            std::map<Model::GroupNode*, std::string> performRenameGroups(const std::string& newName);
            void performUndoRenameGroups(const std::map<Model::GroupNode*, std::string>& newNames);

            void performPushGroup(Model::GroupNode* group);
            void performPopGroup();
        public: // transformation
            /**
             * @return true if the transform was applied, false if can't be applied
             * to everything in the selection, in which case it's the caller's responsibility to restore the modified
             * nodes.
             */
            bool performTransform(const vm::mat4x4& transform, bool lockTextures);
        public: // entity attributes
            using EntityAttributeSnapshotMap = std::map<Model::AttributableNode*, std::vector<Model::EntityAttributeSnapshot>>;
            EntityAttributeSnapshotMap performSetAttribute(const std::string& name, const std::string& value);
            EntityAttributeSnapshotMap performSetAttributeForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& name, const std::string& value);
            EntityAttributeSnapshotMap performRemoveAttribute(const std::string& name);
            EntityAttributeSnapshotMap performRemoveAttributeForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& name);
            EntityAttributeSnapshotMap performUpdateSpawnflag(const std::string& name, const size_t flagIndex, const bool setFlag);
            EntityAttributeSnapshotMap performConvertColorRange(const std::string& name, Assets::ColorRange::Type colorRange);
            EntityAttributeSnapshotMap performRenameAttribute(const std::string& oldName, const std::string& newName);
            EntityAttributeSnapshotMap performRenameAttributeForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& oldName, const std::string& newName);
            void restoreAttributes(const EntityAttributeSnapshotMap& attributes);
        public: // brush resizing
            /**
             * Resize the currently selected brushes by translating faces that match the given polygons by the given
             * delta.
             *
             * Returns the new polygons of the translated faces or an empty optional if the operation fails for any of
             * the selected faces. If the operation fails, no brushes will be modified.
             *
             * @param polygons the polygons describing the faces to be translated
             * @param delta the delta vector by which the faces should be translated
             * @return the new polygons or an empty optional if the operation fails
             */
            std::optional<std::vector<vm::polygon3>> performResizeBrushes(const std::vector<vm::polygon3>& polygons, const vm::vec3& delta);
        public: // brush face attributes
            void performMoveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);
            void performRotateTextures(float angle);
            void performShearTextures(const vm::vec2f& factors);
            void performCopyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
            void performChangeBrushFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request);
        public: // vertices
            bool performSnapVertices(FloatType snapTo);
            std::vector<vm::vec3> performMoveVertices(const std::map<Model::BrushNode*, std::vector<vm::vec3>>& vertices, const vm::vec3& delta);
            std::vector<vm::segment3> performMoveEdges(const std::map<Model::BrushNode*, std::vector<vm::segment3>>& edges, const vm::vec3& delta);
            std::vector<vm::polygon3> performMoveFaces(const std::map<Model::BrushNode*, std::vector<vm::polygon3>>& faces, const vm::vec3& delta);
            void performAddVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices);
            void performRemoveVertices(const std::map<Model::BrushNode*, std::vector<vm::vec3>>& vertices);
        public: // snapshots and restoration
            void restoreSnapshot(Model::Snapshot* snapshot);
        public: // entity definition file management
            void performSetEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);
        public: // texture collection management
            void performSetTextureCollections(const std::vector<IO::Path>& paths);
        public: // mods management
            void performSetMods(const std::vector<std::string>& mods);
        private:
            void doSetIssueHidden(Model::Issue* issue, bool hidden) override;
        public: // modification count
            void incModificationCount(size_t delta = 1);
            void decModificationCount(size_t delta = 1);
        private: // notification
            void bindObservers();
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
        private: // implement MapDocument interface
            bool doCanUndoCommand() const override;
            bool doCanRedoCommand() const override;
            const std::string& doGetUndoCommandName() const override;
            const std::string& doGetRedoCommandName() const override;
            void doUndoCommand() override;
            void doRedoCommand() override;

            void doStartTransaction(const std::string& name) override;
            void doCommitTransaction() override;
            void doRollbackTransaction() override;

            std::unique_ptr<CommandResult> doExecute(std::unique_ptr<Command>&& command) override;
            std::unique_ptr<CommandResult> doExecuteAndStore(std::unique_ptr<UndoableCommand>&& command) override;
        };
    }
}

#endif /* defined(TrenchBroom_MapDocumentCommandFacade) */
