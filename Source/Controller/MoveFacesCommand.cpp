/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveFacesCommand.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveFacesCommand::performDo() {
            if (!canDo())
                return false;
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            
            BrushFacesMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::FaceList& faces = it->second;
                brush->moveFaces(faces, m_delta);
            }

            // now the faces are not identical to the originally moved faces anymore
            
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool MoveFacesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        MoveFacesCommand::MoveFacesCommand(Model::MapDocument& document, const wxString& name, const Model::VertexToFacesMap& brushFaces, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_delta(delta) {
            Model::VertexToFacesMap::const_iterator mapIt, mapEnd;
            for (mapIt = brushFaces.begin(), mapEnd = brushFaces.end(); mapIt != mapEnd; ++mapIt) {
                const Model::FaceList& faces = mapIt->second;
                Model::FaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    Model::Face* face = *faceIt;
                    Model::Brush* brush = face->brush();
                    
                    BrushFacesMapInsertResult result = m_brushFaces.insert(BrushFacesMapEntry(brush, Model::EmptyFaceList));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(face);
                    m_faces.push_back(face);
                }
            }
            
            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushFaces.size());
        }

        MoveFacesCommand* MoveFacesCommand::moveFaces(Model::MapDocument& document, const Model::VertexToFacesMap& brushFaces, const Vec3f& delta) {
            return new MoveFacesCommand(document, brushFaces.size() == 1 ? wxT("Move Face") : wxT("Move Faces"), brushFaces, delta);
        }

        bool MoveFacesCommand::canDo() const {
            BrushFacesMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::FaceList& faces = it->second;
                if (!brush->canMoveFaces(faces, m_delta))
                    return false;
            }
            return true;
        }
    }
}
