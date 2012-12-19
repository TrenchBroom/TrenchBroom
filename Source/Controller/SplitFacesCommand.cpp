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

#include "SplitFacesCommand.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool SplitFacesCommand::performDo() {
            m_vertices.clear();
            Model::FaceList::const_iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                Model::Face* face = *fIt;
                Model::Brush* brush = face->brush();
                if (!brush->canSplitFace(face, m_delta))
                    return false;
            }
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);

            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                Model::Face* face = *fIt;
                Model::Brush* brush = face->brush();
                Vec3f newVertexPosition = brush->splitFace(face, m_delta);
                m_vertices.insert(newVertexPosition);
            }

            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool SplitFacesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            
            m_vertices.clear();
            return true;
        }

        SplitFacesCommand::SplitFacesCommand(Model::MapDocument& document, const wxString& name, const Model::FaceList& faces, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_faces(faces),
        m_delta(delta) {
            Model::FaceList::const_iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                Model::Face* face = *fIt;
                m_brushes.push_back(face->brush());
            }
        }

        SplitFacesCommand* SplitFacesCommand::splitFaces(Model::MapDocument& document, const Model::FaceList& faces, const Vec3f& delta) {
            return new SplitFacesCommand(document, faces.size() == 1 ? wxT("Split Face") : wxT("Split Faces"), faces, delta);
        }
    }
}