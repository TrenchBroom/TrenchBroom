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

#include "ResizeBrushesCommand.h"

#include "View/MapDocumentCommandFacade.h"

#include "TrenchBroom.h"
#include <vecmath/vec.h>
#include <vecmath/polygon.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ResizeBrushesCommand::Type = Command::freeType();

        ResizeBrushesCommand::Ptr ResizeBrushesCommand::resize(const vm::polygon3::List& faces, const vm::vec3& delta) {
            return Ptr(new ResizeBrushesCommand(faces, delta));
        }

        ResizeBrushesCommand::ResizeBrushesCommand(const vm::polygon3::List& faces, const vm::vec3& delta) :
        SnapshotCommand(Type, "Resize Brushes"),
        m_faces(faces),
        m_delta(delta) {}
        
        bool ResizeBrushesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_newFaces = document->performResizeBrushes(m_faces, m_delta);
            return !m_newFaces.empty();
        }
        

        bool ResizeBrushesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool ResizeBrushesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ResizeBrushesCommand* other = static_cast<ResizeBrushesCommand*>(command.get());
            if (other->m_faces == m_newFaces) {
                m_newFaces = other->m_newFaces;
                m_delta = m_delta + other->m_delta;
                return true;
            } else {
                return false;
            }
        }
    }
}
