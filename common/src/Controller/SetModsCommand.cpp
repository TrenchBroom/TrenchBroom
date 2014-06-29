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

#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType SetModsCommand::Type = Command::freeType();

        SetModsCommand::Ptr SetModsCommand::setMods(View::MapDocumentWPtr document, const StringList& mods) {
            return Ptr(new SetModsCommand(document, mods));
        }
        
        SetModsCommand::SetModsCommand(View::MapDocumentWPtr document, const StringList& mods) :
        Command(Type, "Set Mods", true, true),
        m_document(document),
        m_newMods(mods) {}
        
        bool SetModsCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::Entity* worldspawn = document->worldspawn();
            document->objectWillChangeNotifier(worldspawn);
            m_oldMods = document->mods();
            worldspawn->addOrUpdateProperty(Model::PropertyKeys::Mods, StringUtils::join(m_newMods, ';'));
            document->objectDidChangeNotifier(worldspawn);
            document->modsDidChangeNotifier();
            return true;
        }
        
        bool SetModsCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::Entity* worldspawn = document->worldspawn();
            document->objectWillChangeNotifier(worldspawn);
            worldspawn->addOrUpdateProperty(Model::PropertyKeys::Mods, StringUtils::join(m_oldMods, ';'));
            document->objectDidChangeNotifier(worldspawn);
            document->modsDidChangeNotifier();
            return true;
        }

        bool SetModsCommand::doCollateWith(Command::Ptr command) {
            return false;
        }
    }
}
