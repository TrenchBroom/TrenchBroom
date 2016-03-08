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

#include "TextureCollectionCommand.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType TextureCollectionCommand::Type = Command::freeType();
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::add(const String& collectionName) {
            return Ptr(new TextureCollectionCommand("Add Texture Collection", Action_Add, StringList(1, collectionName)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::remove(const StringList& collectionNames) {
            const String name = StringUtils::safePlural(collectionNames.size(), "Remove Texture Collection", "Remove Texture Collections");
            return Ptr(new TextureCollectionCommand(name, Action_Remove, collectionNames));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveUp(const String& collectionName) {
            return Ptr(new TextureCollectionCommand("Move Texture Collection Up", Action_MoveUp, StringList(1, collectionName)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveDown(const String& collectionName) {
            return Ptr(new TextureCollectionCommand("Move Texture Collection Down", Action_MoveDown, StringList(1, collectionName)));
        }

        TextureCollectionCommand::TextureCollectionCommand(const String& name, const Action action, const StringList& collectionNames) :
        DocumentCommand(Type, name),
        m_action(action),
        m_collectionNames(collectionNames) {
            switch (m_action) {
                case Action_Add:
                case Action_MoveUp:
                case Action_MoveDown:
                    assert(m_collectionNames.size() == 1);
                    break;
                case Action_Remove:
                    break;
            }
        }
        
        bool TextureCollectionCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performAddExternalTextureCollections(m_collectionNames);
                    break;
                case Action_Remove:
                    document->performRemoveExternalTextureCollections(m_collectionNames);
                    break;
                case Action_MoveUp:
                    document->performMoveExternalTextureCollectionUp(m_collectionNames.front());
                    break;
                case Action_MoveDown:
                    document->performMoveExternalTextureCollectionDown(m_collectionNames.front());
                    break;
            }
            return true;
        }
        
        bool TextureCollectionCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performRemoveExternalTextureCollections(m_collectionNames);
                    break;
                case Action_Remove:
                    document->performAddExternalTextureCollections(m_collectionNames);
                    break;
                case Action_MoveUp:
                    document->performMoveExternalTextureCollectionDown(m_collectionNames.front());
                    break;
                case Action_MoveDown:
                    document->performMoveExternalTextureCollectionUp(m_collectionNames.front());
                    break;
            }
            return true;
        }
        
        bool TextureCollectionCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool TextureCollectionCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
