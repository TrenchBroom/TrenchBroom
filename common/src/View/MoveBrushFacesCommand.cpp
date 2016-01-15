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


#include "MoveBrushFacesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushFacesCommand::Type = Command::freeType();

        MoveBrushFacesCommand::Ptr MoveBrushFacesCommand::move(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            Polygon3::List facePositions;
            extractFaceMap(faces, brushes, brushFaces, facePositions);
            
            return Ptr(new MoveBrushFacesCommand(brushes, brushFaces, facePositions, delta));
        }

        MoveBrushFacesCommand::MoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const Polygon3::List& facePositions, const Vec3& delta) :
        VertexCommand(Type, "Move faces", brushes),
        m_faces(faces),
        m_oldFacePositions(facePositions),
        m_delta(delta) {
            assert(!m_delta.null());
        }
        
        bool MoveBrushFacesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const BBox3& worldBounds = document->worldBounds();
            Model::BrushFacesMap::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Polygon3::List& faces = it->second;
                if (!brush->canMoveFaces(worldBounds, faces, m_delta))
                    return false;
            }
            return true;
        }
        
        bool MoveBrushFacesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newFacePositions = document->performMoveFaces(m_faces, m_delta);
            return true;
        }
        
        void MoveBrushFacesCommand::doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectFaceHandles(m_newFacePositions);
        }
        
        void MoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectFaceHandles(m_oldFacePositions);
        }
        
        bool MoveBrushFacesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushFacesCommand* other = static_cast<MoveBrushFacesCommand*>(command.get());
            
            if (!VectorUtils::equals(m_newFacePositions, other->m_oldFacePositions))
                return false;
            
            m_newFacePositions = other->m_newFacePositions;
            m_delta += other->m_delta;
            
            return true;
        }
    }
}
