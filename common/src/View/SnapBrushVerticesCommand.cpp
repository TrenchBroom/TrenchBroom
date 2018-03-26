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

#include "SnapBrushVerticesCommand.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snap(FloatType snapTo) {
            return Ptr(new SnapBrushVerticesCommand(snapTo));
        }
        
        SnapBrushVerticesCommand::SnapBrushVerticesCommand(const FloatType snapTo) :
        DocumentCommand(Type, "Snap Brush Vertices"),
        m_snapTo(snapTo),
        m_snapshot(nullptr) {}
        
        SnapBrushVerticesCommand::~SnapBrushVerticesCommand() {
            delete m_snapshot;
        }

        bool SnapBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            assert(m_snapshot == nullptr);
            m_snapshot = document->performSnapVertices(m_snapTo);
            return true;
        }
        
        bool SnapBrushVerticesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            ensure(m_snapshot != nullptr, "snapshot is null");
            document->restoreSnapshot(m_snapshot);
            delete m_snapshot;
            m_snapshot = nullptr;
            return true;
        }
        
        bool SnapBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }

        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            SnapBrushVerticesCommand* other = static_cast<SnapBrushVerticesCommand*>(command.get());
            return other->m_snapTo == m_snapTo;
        }
    }
}
