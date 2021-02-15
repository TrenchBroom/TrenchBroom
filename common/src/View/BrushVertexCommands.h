/*
 Copyright (C) 2020 Kristian Duske

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

#include "Macros.h"
#include "View/SwapNodeContentsCommand.h"

#include <vecmath/forward.h>
#include <vecmath/polygon.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocument;
        class VertexHandleManagerBase;
        template <typename H> class VertexHandleManagerBaseT;

        class BrushVertexCommandBase : public SwapNodeContentsCommand {
        protected:
            BrushVertexCommandBase(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            virtual std::unique_ptr<CommandResult> createCommandResult(std::unique_ptr<CommandResult> swapResult);
        public:
            void removeHandles(VertexHandleManagerBase& manager);
            void addHandles(VertexHandleManagerBase& manager);
        public:
            virtual void selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
            virtual void selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;

            deleteCopyAndMove(BrushVertexCommandBase)
        };

        class BrushVertexCommandResult : public CommandResult {
        private:
            bool m_hasRemainingVertices;
        public:
            BrushVertexCommandResult(bool success, bool hasRemainingVertices);

            bool hasRemainingVertices() const;
        };

        class BrushVertexCommand : public BrushVertexCommandBase {
        public:
            static const CommandType Type;
        private:
            std::vector<vm::vec3> m_oldVertexPositions;
            std::vector<vm::vec3> m_newVertexPositions;
        public:
            BrushVertexCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes, std::vector<vm::vec3> oldVertexPositions, std::vector<vm::vec3> newVertexPositions, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
        private:
            std::unique_ptr<CommandResult> createCommandResult(std::unique_ptr<CommandResult> swapResult) override;

            bool doCollateWith(UndoableCommand* command) override;

            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;

            deleteCopyAndMove(BrushVertexCommand)
        };

        class BrushEdgeCommand : public BrushVertexCommandBase {
        public:
            static const CommandType Type;
        private:
            std::vector<vm::segment3> m_oldEdgePositions;
            std::vector<vm::segment3> m_newEdgePositions;
        public:
            BrushEdgeCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes, std::vector<vm::segment3> oldEdgePositions, std::vector<vm::segment3> newEdgePositions, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
        private:
            bool doCollateWith(UndoableCommand* command) override;

            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;

            deleteCopyAndMove(BrushEdgeCommand)
        };

        class BrushFaceCommand : public BrushVertexCommandBase {
        public:
            static const CommandType Type;
        private:
            std::vector<vm::polygon3> m_oldFacePositions;
            std::vector<vm::polygon3> m_newFacePositions;
        public:
            BrushFaceCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes, std::vector<vm::polygon3> oldFacePositions, std::vector<vm::polygon3> newFacePositions, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
        private:
            bool doCollateWith(UndoableCommand* command) override;

            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;

            deleteCopyAndMove(BrushFaceCommand)
        };
    }
}
