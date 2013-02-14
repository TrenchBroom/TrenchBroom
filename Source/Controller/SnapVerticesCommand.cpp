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

#include "SnapVerticesCommand.h"

#include "Model/Brush.h"
#include "Utility/Grid.h"

namespace TrenchBroom {
    namespace Controller {
        bool SnapVerticesCommand::performDo() {
            
            makeSnapshots(m_brushes);
            document().brushesWillChange(m_brushes);
            
            Model::BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush& brush = **it;
                brush.snap(m_snapTo);
            }
            
            document().brushesDidChange(m_brushes);
            return true;
        }
        
        bool SnapVerticesCommand::performUndo() {
            document().brushesWillChange(m_brushes);
            restoreSnapshots(m_brushes);
            document().brushesDidChange(m_brushes);
            return true;
        }

        SnapVerticesCommand::SnapVerticesCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, unsigned int snapTo) :
        SnapshotCommand(Command::SnapVertices, document, name),
        m_brushes(brushes),
        m_snapTo(snapTo) {}

        SnapVerticesCommand* SnapVerticesCommand::correct(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new SnapVerticesCommand(document, wxT("Correct Vertices"), brushes, 0);
        }
        
        SnapVerticesCommand* SnapVerticesCommand::snapTo1(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new SnapVerticesCommand(document, wxT("Snap Vertices"), brushes, 1);
        }
        
        SnapVerticesCommand* SnapVerticesCommand::snapToGrid(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new SnapVerticesCommand(document, wxT("Snap Vertices to Grid"), brushes, document.grid().actualSize());
        }
    }
}
