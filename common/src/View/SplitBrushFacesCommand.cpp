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

#include "SplitBrushFacesCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SplitBrushFacesCommand::Type = Command::freeType();
        
        SplitBrushFacesCommand::Ptr SplitBrushFacesCommand::split(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            Polygon3::List facePositions;
            extractFaceMap(faces, brushes, brushFaces, facePositions);
            
            return Ptr(new SplitBrushFacesCommand(brushes, brushFaces, facePositions, delta));
        }
        
        SplitBrushFacesCommand::SplitBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const Polygon3::List& facePositions, const Vec3& delta) :
        VertexCommand(Type, "Split faces", brushes),
        m_faces(faces),
        m_oldFacePositions(facePositions),
        m_delta(delta) {
            assert(!m_delta.null());
        }
        
        bool SplitBrushFacesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const BBox3& worldBounds = document->worldBounds();
            Model::BrushFacesMap::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Polygon3::List& faces = it->second;
                for (size_t i = 0; i < faces.size(); ++i) {
                    const Polygon3& face = faces[i];
                    if (!brush->canSplitFace(worldBounds, face, m_delta))
                        return false;
                }
            }
            return true;
        }
        
        bool SplitBrushFacesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performSplitFaces(m_faces, m_delta);
            return true;
        }
        
        void SplitBrushFacesCommand::doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectVertexHandles(m_newVertexPositions);
        }
        
        void SplitBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectFaceHandles(m_oldFacePositions);
        }
        
        bool SplitBrushFacesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
