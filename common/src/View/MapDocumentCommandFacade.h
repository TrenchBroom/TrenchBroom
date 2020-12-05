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
#include "Model/NodeContents.h"
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
        public: // swapping node contents
            void performSwapNodeContents(std::vector<std::pair<Model::Node*, Model::NodeContents>>& nodesToSwap);
        public: // Node Visibility
            std::map<Model::Node*, Model::VisibilityState> setVisibilityState(const std::vector<Model::Node*>& nodes, Model::VisibilityState visibilityState);
            std::map<Model::Node*, Model::VisibilityState> setVisibilityEnsured(const std::vector<Model::Node*>& nodes);
            void restoreVisibilityState(const std::map<Model::Node*, Model::VisibilityState>& nodes);
            std::map<Model::Node*, Model::LockState> setLockState(const std::vector<Model::Node*>& nodes, Model::LockState lockState);
            void restoreLockState(const std::map<Model::Node*, Model::LockState>& nodes);
        public: // layers
            using MapDocument::performSetCurrentLayer;
        public:
            void performPushGroup(Model::GroupNode* group);
            void performPopGroup();
        public: // brush face attributes
            void performMoveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);
            void performRotateTextures(float angle);
            void performShearTextures(const vm::vec2f& factors);
            void performCopyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
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

