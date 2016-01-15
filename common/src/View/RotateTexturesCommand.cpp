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

#include "RotateTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RotateTexturesCommand::Type = Command::freeType();
        
        RotateTexturesCommand::Ptr RotateTexturesCommand::rotate(const float angle) {
            return Ptr(new RotateTexturesCommand(angle));
        }
        
        RotateTexturesCommand::RotateTexturesCommand(const float angle) :
        DocumentCommand(Type, "Move Textures"),
        m_angle(angle) {}
        
        bool RotateTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return rotateTextures(document, m_angle);
        }
        
        bool RotateTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            return rotateTextures(document, -m_angle);
        }
        
        bool RotateTexturesCommand::rotateTextures(MapDocumentCommandFacade* document, const float angle) const {
            document->performRotateTextures(angle);
            return true;
        }
        
        bool RotateTexturesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return true;
        }
        
        UndoableCommand::Ptr RotateTexturesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new RotateTexturesCommand(*this));
        }
        
        bool RotateTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            const RotateTexturesCommand* other = static_cast<RotateTexturesCommand*>(command.get());
            
            m_angle += other->m_angle;
            return true;
        }
    }
}
