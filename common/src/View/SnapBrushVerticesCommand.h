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

#ifndef TrenchBroom_SnapBrushVerticesCommand
#define TrenchBroom_SnapBrushVerticesCommand

#include "FloatType.h"
#include "Macros.h"
#include "View/SnapshotCommand.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace View {
        /**
         * Snaps all vertices of all selected brushes.
         */
        class SnapBrushVerticesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
        private:
            FloatType m_snapTo;
        public:
            static std::unique_ptr<SnapBrushVerticesCommand> snap(FloatType snapTo);

            explicit SnapBrushVerticesCommand(FloatType snapTo);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SnapBrushVerticesCommand)
        };

        /**
         * Snaps the given vertices of the given brushes.
         */
        class SnapSpecificBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            FloatType m_snapTo;
            BrushVerticesMap m_vertices;
            std::vector<vm::vec3> m_oldVertexPositions;
            std::vector<vm::vec3> m_newVertexPositions;
        public:
            static std::unique_ptr<SnapSpecificBrushVerticesCommand> snap(FloatType snapTo, const VertexToBrushesMap& vertices);

            SnapSpecificBrushVerticesCommand(FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;

            deleteCopyAndMove(SnapSpecificBrushVerticesCommand)
        };

        /**
         * Snaps the given edges of the given brushes.
         */
        class SnapSpecificBrushEdgesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            FloatType m_snapTo;
            BrushEdgesMap m_edges;
            std::vector<vm::segment3> m_oldEdgePositions;
            std::vector<vm::segment3> m_newEdgePositions;
        public:
            static std::unique_ptr<SnapSpecificBrushEdgesCommand> snap(FloatType snapTo, const EdgeToBrushesMap& vertices);

            SnapSpecificBrushEdgesCommand(FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;

            deleteCopyAndMove(SnapSpecificBrushEdgesCommand)
        };

        /**
         * Snaps the given faces of the given brushes.
         */
        class SnapSpecificBrushFacesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            FloatType m_snapTo;
            BrushFacesMap m_faces;
            std::vector<vm::polygon3> m_oldFacePositions;
            std::vector<vm::polygon3> m_newFacePositions;
        public:
            static std::unique_ptr<SnapSpecificBrushFacesCommand> snap(FloatType snapTo, const FaceToBrushesMap& vertices);

            SnapSpecificBrushFacesCommand(FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushFacesMap& faces, const std::vector<vm::polygon3>& facePositions);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;

            deleteCopyAndMove(SnapSpecificBrushFacesCommand)
        };
    }
}

#endif /* defined(TrenchBroom_SnapBrushVerticesCommand) */
