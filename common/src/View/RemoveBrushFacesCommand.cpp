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

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManagerOld.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushFacesCommand::Type = Command::freeType();

        RemoveBrushFacesCommand::Ptr RemoveBrushFacesCommand::remove(const Model::VertexToFacesMap& faces) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            Polygon3::List facePositions;
            
            extractFaceMap(faces, brushes, brushFaces, facePositions);
            const Model::BrushVerticesMap brushVertices = brushVertexMap(brushFaces);
            
            return Ptr(new RemoveBrushFacesCommand(brushes, brushVertices, facePositions));
        }

        RemoveBrushFacesCommand::RemoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Polygon3::List& facePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Faces", brushes, vertices),
        m_oldFacePositions(facePositions) {}

        void RemoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerOld& manager, const Model::BrushList& brushes) {
            manager.selectFaceHandles(m_oldFacePositions);
        }
        
        void RemoveBrushFacesCommand::doSelectOldHandlePositions(FaceHandleManager& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
