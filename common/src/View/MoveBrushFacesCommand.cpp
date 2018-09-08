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


#include "MoveBrushFacesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushFacesCommand::Type = Command::freeType();

        MoveBrushFacesCommand::Ptr MoveBrushFacesCommand::move(const Model::FaceToBrushesMap& faces, const vm::vec3& delta) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            polygon3::List facePositions;
            extractFaceMap(faces, brushes, brushFaces, facePositions);
            
            return Ptr(new MoveBrushFacesCommand(brushes, brushFaces, facePositions, delta));
        }

        MoveBrushFacesCommand::MoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const polygon3::List& facePositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Faces", brushes),
        m_faces(faces),
        m_oldFacePositions(facePositions),
        m_delta(delta) {
            assert(!isZero(m_delta));
        }
        
        bool MoveBrushFacesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_faces) {
                Model::Brush* brush = entry.first;
                const polygon3::List& faces = entry.second;
                if (!brush->canMoveFaces(worldBounds, faces, m_delta))
                    return false;
            }
            return true;
        }
        
        bool MoveBrushFacesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newFacePositions = document->performMoveFaces(m_faces, m_delta);
            return true;
        }

        bool MoveBrushFacesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushFacesCommand* other = static_cast<MoveBrushFacesCommand*>(command.get());

            if (!VectorUtils::equals(m_newFacePositions, other->m_oldFacePositions))
                return false;

            m_newFacePositions = other->m_newFacePositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }


        void MoveBrushFacesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<polygon3>& manager) const {
            manager.select(std::begin(m_newFacePositions), std::end(m_newFacePositions));
        }
        
        void MoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<polygon3>& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
