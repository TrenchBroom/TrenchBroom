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
            if (!canDo())
                return false;
            
            m_vertices.clear();
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);

            BrushFaceMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::FaceInfo& faceInfo = it->second;
                Vec3f newVertexPosition = brush->splitFace(faceInfo, m_delta);
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
        m_delta(delta) {
            Model::FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::Face& face = **it;
                Model::FaceInfo faceInfo = face.faceInfo();
                Model::Brush* brush = face.brush();
                
                BrushFaceMapInsertResult result = m_brushFaces.insert(BrushFaceMapEntry(brush, faceInfo));
                assert(result.second);
                m_brushes.push_back(brush);
            }
        }

        SplitFacesCommand* SplitFacesCommand::splitFaces(Model::MapDocument& document, const Model::FaceList& faces, const Vec3f& delta) {
            return new SplitFacesCommand(document, faces.size() == 1 ? wxT("Split Face") : wxT("Split Faces"), faces, delta);
        }

        bool SplitFacesCommand::canDo() const {
            BrushFaceMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                const Model::Brush* brush = it->first;
                const Model::FaceInfo& faceInfo = it->second;
                if (!brush->canSplitFace(faceInfo, m_delta))
                    return false;
            }
            return true;
        }
    }
}
