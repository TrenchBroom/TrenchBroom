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

#include "Controller/VertexHandleManager.h"
#include "MoveFacesCommand.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveFacesCommand::performDo() {
            if (!canDo())
                return false;

            m_handleManager.remove(m_brushes);
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            m_facesAfter.clear();

            Model::BrushFacesMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::FaceInfoList& faceInfos = it->second;
                const Model::FaceInfoList newFaces = brush->moveFaces(faceInfos, m_delta);
                m_facesAfter.insert(m_facesAfter.end(), newFaces.begin(), newFaces.end());
            }

            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectFaceHandles(m_facesAfter);

            return true;
        }

        bool MoveFacesCommand::performUndo() {
            m_handleManager.remove(m_brushes);
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectFaceHandles(m_facesBefore);
            
            return true;
        }

        MoveFacesCommand::MoveFacesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta) :
        SnapshotCommand(Command::MoveVertices, document, name),
        m_handleManager(handleManager),
        m_delta(delta) {
            const Model::VertexToFacesMap& brushFaces = m_handleManager.selectedFaceHandles();
            Model::VertexToFacesMap::const_iterator mapIt, mapEnd;
            for (mapIt = brushFaces.begin(), mapEnd = brushFaces.end(); mapIt != mapEnd; ++mapIt) {
                const Model::FaceList& faces = mapIt->second;
                Model::FaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    Model::Face* face = *faceIt;
                    Model::Brush* brush = face->brush();
                    const Model::FaceInfo faceInfo = face->faceInfo();

                    Model::BrushFacesMapInsertResult result = m_brushFaces.insert(Model::BrushFacesMapEntry(brush, Model::FaceInfoList()));
                    if (result.second)
                        m_brushes.push_back(brush);
                    result.first->second.push_back(faceInfo);
                    m_facesBefore.push_back(faceInfo);
                }
            }

            assert(!m_brushes.empty());
            assert(m_brushes.size() == m_brushFaces.size());
        }

        MoveFacesCommand* MoveFacesCommand::moveFaces(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta) {
            return new MoveFacesCommand(document, handleManager.selectedFaceHandles().size() == 1 ? wxT("Move Face") : wxT("Move Faces"), handleManager, delta);
        }

        bool MoveFacesCommand::canDo() const {
            Model::BrushFacesMap::const_iterator it, end;
            for (it = m_brushFaces.begin(), end = m_brushFaces.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                const Model::FaceInfoList& faces = it->second;
                if (!brush->canMoveFaces(faces, m_delta))
                    return false;
            }
            return true;
        }
    }
}
