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

#include "MoveTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveTexturesCommand::Type = Command::freeType();

        MoveTexturesCommand::Ptr MoveTexturesCommand::move(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) {
            return Ptr(new MoveTexturesCommand(cameraUp, cameraRight, delta));
        }

        MoveTexturesCommand::MoveTexturesCommand(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) :
        DocumentCommand(Type, "Move textures"),
        m_cameraUp(cameraUp),
        m_cameraRight(cameraRight),
        m_delta(delta) {}
        
        bool MoveTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            moveTextures(document, m_delta);
            return true;
        }
        
        bool MoveTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            moveTextures(document, -m_delta);
            return true;
        }
        
        void MoveTexturesCommand::moveTextures(MapDocumentCommandFacade* document, const Vec2f& delta) const {
            document->performMoveTextures(m_cameraUp, m_cameraRight, delta);
        }

        bool MoveTexturesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return true;
        }
        
        UndoableCommand::Ptr MoveTexturesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new MoveTexturesCommand(*this));
        }
        
        bool MoveTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            const MoveTexturesCommand* other = static_cast<MoveTexturesCommand*>(command.get());
            
            if (other->m_cameraUp != m_cameraUp ||
                other->m_cameraRight != m_cameraRight)
                return false;
            
            m_delta += other->m_delta;
            return true;
        }
    }
}
