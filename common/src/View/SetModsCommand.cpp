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

#include "SetModsCommand.h"

#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetModsCommand::Type = Command::freeType();

        SetModsCommand::Ptr SetModsCommand::set(const StringList& mods) {
            return Ptr(new SetModsCommand("Set Mods", mods));
        }

        SetModsCommand::SetModsCommand(const String& name, const StringList& mods) :
        DocumentCommand(Type, name),
        m_newMods(mods) {}
        
        bool SetModsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldMods = document->mods();
            document->performSetMods(m_newMods);
            return true;
        }
        
        bool SetModsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetMods(m_oldMods);
            return true;
        }
        
        bool SetModsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool SetModsCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
