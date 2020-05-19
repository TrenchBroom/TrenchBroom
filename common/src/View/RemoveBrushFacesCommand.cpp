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

#include "RemoveBrushFacesCommand.h"

#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

#include <vecmath/polygon.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushFacesCommand::Type = Command::freeType();

        std::unique_ptr<RemoveBrushFacesCommand> RemoveBrushFacesCommand::remove(const FaceToBrushesMap& faces) {
            std::vector<Model::BrushNode*> brushes;
            BrushFacesMap brushFaces;
            std::vector<vm::polygon3> facePositions;

            extractFaceMap(faces, brushes, brushFaces, facePositions);
            const BrushVerticesMap brushVertices = brushVertexMap(brushFaces);

            return std::make_unique<RemoveBrushFacesCommand>(brushes, brushVertices, facePositions);
        }

        RemoveBrushFacesCommand::RemoveBrushFacesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::polygon3>& facePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Faces", brushes, vertices),
        m_oldFacePositions(facePositions) {}

        void RemoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
