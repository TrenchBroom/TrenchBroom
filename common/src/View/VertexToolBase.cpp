/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "VertexToolBase.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        VertexToolBase::VertexToolBase(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_changeCount(0),
        m_ignoreChangeNotifications(false),
        m_dragging(false) {}
        
        VertexToolBase::~VertexToolBase() {}

        const Grid& VertexToolBase::grid() const {
            return lock(m_document)->grid();
        }

        const Model::BrushList& VertexToolBase::selectedBrushes() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().brushes();
        }

        bool VertexToolBase::doActivate() {
            m_changeCount = 0;
            return true;
        }
        
        bool VertexToolBase::doDeactivate() {
            /*
             if (m_changeCount > 0) {
             RebuildBrushGeometryCommand* command = RebuildBrushGeometryCommand::rebuildGeometry(document, document.editStateManager().selectedBrushes(), m_changeCount);
             submitCommand(command);
             }
             */
            return true;
        }
    }
}
