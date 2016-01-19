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

#include "ShearTexturesCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ShearTexturesCommand::Type = Command::freeType();
        
        ShearTexturesCommand::Ptr ShearTexturesCommand::shear(const Vec2f& factors) {
            return Ptr(new ShearTexturesCommand(factors));
        }

        ShearTexturesCommand::ShearTexturesCommand(const Vec2f& factors) :
        DocumentCommand(Type, "Shear Textures"),
        m_factors(factors) {
            assert(factors.x() != 0.0f || factors.y() != 0.0f);
        }
        
        bool ShearTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return shearTextures(document, m_factors);
        }
        
        bool ShearTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            return shearTextures(document, -m_factors);
        }
        
        bool ShearTexturesCommand::shearTextures(MapDocumentCommandFacade* document, const Vec2f& factors) {
            document->performShearTextures(factors);
            return true;
        }

        bool ShearTexturesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return true;
        }
        
        UndoableCommand::Ptr ShearTexturesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new ShearTexturesCommand(*this));
        }
        
        bool ShearTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ShearTexturesCommand* other = static_cast<ShearTexturesCommand*>(command.get());
            m_factors += other->m_factors;
            return true;
        }
    }
}
