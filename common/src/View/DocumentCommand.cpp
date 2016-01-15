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

#include "DocumentCommand.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        DocumentCommand::DocumentCommand(const CommandType type, const String& name) :
        UndoableCommand(type, name),
        m_modificationCount(1) {}
        
        DocumentCommand::~DocumentCommand() {}

        bool DocumentCommand::performDo(MapDocumentCommandFacade* document) {
            if (UndoableCommand::performDo(document)) {
                document->incModificationCount(m_modificationCount);
                return true;
            }
            return false;
        }
        
        bool DocumentCommand::performUndo(MapDocumentCommandFacade* document) {
            if (UndoableCommand::performUndo(document)) {
                document->decModificationCount(m_modificationCount);
                return true;
            }
            return false;
        }
        
        bool DocumentCommand::collateWith(UndoableCommand::Ptr command) {
            if (UndoableCommand::collateWith(command)) {
                m_modificationCount += command->documentModificationCount();
                return true;
            }
            return false;
        }

        size_t DocumentCommand::documentModificationCount() const {
            return m_modificationCount;
        }
    }
}
