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
#include "Macros.h"
#include "View/VertexCommand.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MoveBrushFacesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            BrushFacesMap m_faces;
            std::vector<vm::polygon3> m_oldFacePositions;
            std::vector<vm::polygon3> m_newFacePositions;
            vm::vec3 m_delta;
        public:
            static std::unique_ptr<MoveBrushFacesCommand> move(const FaceToBrushesMap& faces, const vm::vec3& delta);

            MoveBrushFacesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushFacesMap& faces, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;

            deleteCopyAndMove(MoveBrushFacesCommand)
        };
    }
}

