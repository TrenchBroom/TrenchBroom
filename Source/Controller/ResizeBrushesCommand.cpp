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

#include "ResizeBrushesCommand.h"

#include "Model/Brush.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Controller {
        bool ResizeBrushesCommand::performDo() {
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& face = **faceIt;
                Model::Brush& brush = *face.brush();
                if (!brush.canMoveBoundary(face, m_distance))
                    return false;
            }
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& face = **faceIt;
                Model::Brush& brush = *face.brush();
                brush.moveBoundary(face, m_distance, m_lockTextures);
            }
            
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool ResizeBrushesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            return true;
        }

        ResizeBrushesCommand::ResizeBrushesCommand(Model::MapDocument& document, const wxString& name, const Model::FaceList& faces, const Model::BrushList& brushes, float distance, bool lockTextures) :
        SnapshotCommand(Command::ResizeBrushes, document, name),
        m_faces(faces),
        m_brushes(brushes),
        m_distance(distance),
        m_lockTextures(lockTextures) {}

        ResizeBrushesCommand* ResizeBrushesCommand::resizeBrushes(Model::MapDocument& document, const Model::FaceList& faces, float distance, bool lockTextures) {
            Model::BrushSet brushSet;
            Model::BrushList brushList;
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face* face = *faceIt;
                Model::Brush* brush = face->brush();
                if (brushSet.insert(brush).second)
                    brushList.push_back(brush);
            }
            
            assert(brushSet.size() == brushList.size());
            
            wxString name = brushList.size() == 1 ? wxT("Resize Brush") : wxT("Resize Brushes");
            return new ResizeBrushesCommand(document, name, faces, brushList, distance, lockTextures);
        }
    }
}