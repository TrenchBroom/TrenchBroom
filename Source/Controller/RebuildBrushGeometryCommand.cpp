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

#include "RebuildBrushGeometryCommand.h"

#include "Model/Brush.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RebuildBrushGeometryCommand::performDo() {
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            
            Model::BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush& brush = **it;
                brush.rebuildGeometry();
            }
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool RebuildBrushGeometryCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        RebuildBrushGeometryCommand::RebuildBrushGeometryCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, size_t precedingChangeCount) :
        SnapshotCommand(Command::RebuildBrushGeometry, document, name),
        m_brushes(brushes),
        m_precedingChangeCount(precedingChangeCount) {}

        RebuildBrushGeometryCommand* RebuildBrushGeometryCommand::rebuildGeometry(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new RebuildBrushGeometryCommand(document, brushes.size() == 1 ? wxT("Rebuild Brush Geometry") : wxT("Rebuild Brush Geometries"), brushes, 0);
        }

        RebuildBrushGeometryCommand* RebuildBrushGeometryCommand::rebuildGeometry(Model::MapDocument& document, const Model::BrushList& brushes, size_t precedingChangeCount) {
            return new RebuildBrushGeometryCommand(document, brushes.size() == 1 ? wxT("Rebuild Brush Geometry") : wxT("Rebuild Brush Geometries"), brushes, precedingChangeCount);
        }
    }
}
