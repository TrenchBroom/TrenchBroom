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

#include "MoveTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveTexturesCommand::Type = Command::freeType();

        std::unique_ptr<MoveTexturesCommand> MoveTexturesCommand::move(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
            return std::make_unique<MoveTexturesCommand>(cameraUp, cameraRight, delta);
        }

        MoveTexturesCommand::MoveTexturesCommand(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) :
        DocumentCommand(Type, "Move Textures"),
        m_cameraUp(cameraUp),
        m_cameraRight(cameraRight),
        m_delta(delta) {}

        std::unique_ptr<CommandResult> MoveTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            moveTextures(document, m_delta);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> MoveTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            moveTextures(document, -m_delta);
            return std::make_unique<CommandResult>(true);
        }

        void MoveTexturesCommand::moveTextures(MapDocumentCommandFacade* document, const vm::vec2f& delta) const {
            document->performMoveTextures(m_cameraUp, m_cameraRight, delta);
        }

        bool MoveTexturesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        std::unique_ptr<UndoableCommand> MoveTexturesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return std::make_unique<MoveTexturesCommand>(m_cameraUp, m_cameraRight, m_delta);
        }

        bool MoveTexturesCommand::doCollateWith(UndoableCommand* command) {
            const MoveTexturesCommand* other = static_cast<MoveTexturesCommand*>(command);

            if (other->m_cameraUp != m_cameraUp ||
                other->m_cameraRight != m_cameraRight)
                return false;

            m_delta = m_delta + other->m_delta;
            return true;
        }
    }
}
