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

#include "RebuildBrushGeometryCommand.h"

#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "View/MapDocument.h"
#include "View/VertexHandleManager.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType RebuildBrushGeometryCommand::Type = Command::freeType();

        RebuildBrushGeometryCommand::Ptr RebuildBrushGeometryCommand::rebuildBrushGeometry(View::MapDocumentWPtr document, const Model::BrushList& brushes) {
            return Ptr(new RebuildBrushGeometryCommand(document, brushes));
        }

        RebuildBrushGeometryCommand::RebuildBrushGeometryCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes) :
        BrushVertexHandleCommand(Type, "Rebuild Brush Geometry", false, true),
        m_document(document),
        m_brushes(brushes) {}
        
        bool RebuildBrushGeometryCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            
            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());

            Model::BrushList::iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                brush->rebuildGeometry(worldBounds);
            }
            
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());
            return true;
        }
        
        void RebuildBrushGeometryCommand::doRemoveBrushes(View::VertexHandleManager& manager) const {
            manager.removeBrushes(m_brushes);
        }
        
        void RebuildBrushGeometryCommand::doAddBrushes(View::VertexHandleManager& manager) const {
            manager.addBrushes(m_brushes);
        }
        
        void RebuildBrushGeometryCommand::doSelectNewHandlePositions(View::VertexHandleManager& manager) const {
        }
        
        void RebuildBrushGeometryCommand::doSelectOldHandlePositions(View::VertexHandleManager& manager) const {
        }
    }
}
