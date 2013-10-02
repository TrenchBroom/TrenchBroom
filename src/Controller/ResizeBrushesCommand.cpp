/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "ResizeBrushesCommand.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType ResizeBrushesCommand::Type = Command::freeType();

        ResizeBrushesCommand::Ptr ResizeBrushesCommand::resizeBrushes(View::MapDocumentPtr document, const Model::BrushFaceList& faces, const Vec3& delta, const bool lockTextures) {
            return Ptr(new ResizeBrushesCommand(document, faces, collectBrushes(faces), delta, lockTextures));
        }
        
        const Model::BrushList& ResizeBrushesCommand::brushes() const {
            return m_brushes;
        }

        ResizeBrushesCommand::ResizeBrushesCommand(View::MapDocumentPtr document, const Model::BrushFaceList& faces, const Model::BrushList& brushes, const Vec3& delta, const bool lockTextures) :
        Command(Type, makeName(brushes), true, true),
        m_document(document),
        m_faces(faces),
        m_brushes(brushes),
        m_delta(delta),
        m_lockTextures(lockTextures) {}
        
        String ResizeBrushesCommand::makeName(const Model::BrushList& brushes) {
            return brushes.size() == 1 ? "Resize Brush" : "Resize Brushes";
        }
        
        Model::BrushList ResizeBrushesCommand::collectBrushes(const Model::BrushFaceList& faces) {
            Model::BrushSet brushSet;
            Model::BrushList brushList;
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                assert(brush != NULL);
                
                if (brushSet.insert(brush).second)
                    brushList.push_back(brush);
            }
            
            return brushList;
        }

        bool ResizeBrushesCommand::doPerformDo() {
            return moveBoundary(m_delta);
        }
        
        bool ResizeBrushesCommand::doPerformUndo() {
            return moveBoundary(-m_delta);
        }

        bool ResizeBrushesCommand::moveBoundary(const Vec3& delta) {
            const BBox3& worldBounds = m_document->worldBounds();
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                if (!brush->canMoveBoundary(worldBounds, *face, delta))
                    return false;
            }

            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                
                m_document->objectWillChangeNotifier(brush);
                brush->moveBoundary(worldBounds, *face, delta, m_lockTextures);
                m_document->objectDidChangeNotifier(brush);
            }
            return true;
        }
    }
}
