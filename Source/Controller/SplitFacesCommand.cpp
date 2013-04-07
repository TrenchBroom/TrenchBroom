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

#include "Controller/VertexHandleManager.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool SplitFacesCommand::performDo() {
            if (!canDo())
                return false;
            
            m_handleManager.remove(m_brushes);
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            m_verticesAfter.clear();

            Model::BrushFacesMap::const_iterator bIt, bEnd;
            for (bIt = m_brushFaces.begin(), bEnd = m_brushFaces.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = bIt->first;
                const Model::FaceInfoList& faceInfos = bIt->second;
                Model::FaceInfoList::const_iterator fIt, fEnd;
                for (fIt = faceInfos.begin(), fEnd = faceInfos.end(); fIt != fEnd; ++fIt) {
                    const Model::FaceInfo& faceInfo = *fIt;
                    Vec3f newVertexPosition = brush->splitFace(faceInfo, m_delta);
                    m_verticesAfter.insert(newVertexPosition);
                }
            }

            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectVertexHandles(m_verticesAfter);

            return true;
        }
        
        bool SplitFacesCommand::performUndo() {
            m_handleManager.remove(m_brushes);
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            m_handleManager.add(m_brushes);
            m_handleManager.selectFaceHandles(m_facesBefore);
            
            return true;
        }

        SplitFacesCommand::SplitFacesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta) :
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

        SplitFacesCommand* SplitFacesCommand::splitFaces(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta) {
            return new SplitFacesCommand(document, handleManager.selectedFaceHandles().size() == 1 ? wxT("Split Face") : wxT("Split Faces"), handleManager, delta);
        }

        bool SplitFacesCommand::canDo() const {
            Model::BrushFacesMap::const_iterator bIt, bEnd;
            for (bIt = m_brushFaces.begin(), bEnd = m_brushFaces.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = bIt->first;
                const Model::FaceInfoList& faceInfos = bIt->second;
                Model::FaceInfoList::const_iterator fIt, fEnd;
                for (fIt = faceInfos.begin(), fEnd = faceInfos.end(); fIt != fEnd; ++fIt) {
                    const Model::FaceInfo& faceInfo = *fIt;
                    if (!brush->canSplitFace(faceInfo, m_delta))
                        return false;
                }
            }
            return true;
        }
    }
}
