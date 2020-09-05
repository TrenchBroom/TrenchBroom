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

#include "FloatType.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/vec.h>
#include <vecmath/polygon.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ResizeBrushesCommand::Type = Command::freeType();

        std::unique_ptr<ResizeBrushesCommand> ResizeBrushesCommand::resize(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) {
            return std::make_unique<ResizeBrushesCommand>(faces, delta);
        }

        ResizeBrushesCommand::ResizeBrushesCommand(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) :
        SnapshotCommand(Type, "Resize Brushes"),
        m_faces(faces),
        m_delta(delta) {}

        std::unique_ptr<CommandResult> ResizeBrushesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_newFaces = document->performResizeBrushes(m_faces, m_delta).value_or(std::vector<vm::polygon3>());
            return std::make_unique<CommandResult>(!m_newFaces.empty());
        }


        bool ResizeBrushesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ResizeBrushesCommand::doCollateWith(UndoableCommand* command) {
            ResizeBrushesCommand* other = static_cast<ResizeBrushesCommand*>(command);
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
