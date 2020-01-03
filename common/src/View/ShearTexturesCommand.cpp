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

#include "ShearTexturesCommand.h"

#include "View/MapDocumentCommandFacade.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ShearTexturesCommand::Type = Command::freeType();

        std::unique_ptr<ShearTexturesCommand> ShearTexturesCommand::shear(const vm::vec2f& factors) {
            return std::make_unique<ShearTexturesCommand>(factors);
        }

        ShearTexturesCommand::ShearTexturesCommand(const vm::vec2f& factors) :
        DocumentCommand(Type, "Shear Textures"),
        m_factors(factors) {
            assert(factors.x() != 0.0f || factors.y() != 0.0f);
        }

        std::unique_ptr<CommandResult> ShearTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return shearTextures(document, m_factors);
        }

        std::unique_ptr<CommandResult> ShearTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            return shearTextures(document, -m_factors);
        }

        std::unique_ptr<CommandResult> ShearTexturesCommand::shearTextures(MapDocumentCommandFacade* document, const vm::vec2f& factors) {
            document->performShearTextures(factors);
            return std::make_unique<CommandResult>(true);
        }

        bool ShearTexturesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        std::unique_ptr<UndoableCommand> ShearTexturesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return std::make_unique<ShearTexturesCommand>(m_factors);
        }

        bool ShearTexturesCommand::doCollateWith(UndoableCommand* command) {
            ShearTexturesCommand* other = static_cast<ShearTexturesCommand*>(command);
            m_factors = m_factors + other->m_factors;
            return true;
        }
    }
}
