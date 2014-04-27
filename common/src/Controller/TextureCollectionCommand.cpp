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

#include "Assets/TextureManager.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Game.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType TextureCollectionCommand::Type = Command::freeType();

        TextureCollectionCommand::Ptr TextureCollectionCommand::add(View::MapDocumentWPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Add Texture Collection",
                                                    Action_Add,
                                                    StringList(1, name)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::remove(View::MapDocumentWPtr document, const StringList& names) {
            return Ptr(new TextureCollectionCommand(document,
                                                    names.size() == 1 ? "Remove Texture Collection" : "Remove Texture Collections",
                                                    Action_Remove,
                                                    names));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveUp(View::MapDocumentWPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collection Up",
                                                    Action_MoveUp,
                                                    StringList(1, name)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveDown(View::MapDocumentWPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collection Down",
                                                    Action_MoveDown,
                                                    StringList(1, name)));
        }

        TextureCollectionCommand::TextureCollectionCommand(View::MapDocumentWPtr document, const String& name, const Action action, const StringList& names) :
        Command(Type, name, true, true),
        m_document(document),
        m_action(action),
        m_names(names) {
            switch (m_action) {
                case Action_Add:
                case Action_MoveUp:
                case Action_MoveDown:
                    assert(m_names.size() == 1);
                    break;
                case Action_Remove:
                    break;
            }
        }
        
        bool TextureCollectionCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::Entity* worldspawn = document->worldspawn();
            
            switch (m_action) {
                case Action_Add:
                    document->addExternalTextureCollections(m_names);
                    break;
                case Action_Remove:
                    document->removeExternalTextureCollections(m_names);
                    break;
                case Action_MoveUp:
                    document->moveExternalTextureCollectionUp(m_names.front());
                    break;
                case Action_MoveDown:
                    document->moveExternalTextureCollectionDown(m_names.front());
                    break;
            }

            document->objectWillChangeNotifier(worldspawn);
            document->updateExternalTextureCollectionProperty();
            document->objectDidChangeNotifier(worldspawn);
            document->textureCollectionsDidChangeNotifier();
            return true;
        }
        
        bool TextureCollectionCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::Entity* worldspawn = document->worldspawn();

            switch (m_action) {
                case Action_Add:
                    document->removeExternalTextureCollections(m_names);
                    break;
                case Action_Remove:
                    document->addExternalTextureCollections(m_names);
                    break;
                case Action_MoveUp:
                    document->moveExternalTextureCollectionDown(m_names.front());
                    break;
                case Action_MoveDown:
                    document->moveExternalTextureCollectionUp(m_names.front());
                    break;
            }

            document->objectWillChangeNotifier(worldspawn);
            document->updateExternalTextureCollectionProperty();
            document->objectDidChangeNotifier(worldspawn);
            document->textureCollectionsDidChangeNotifier();
            return true;
        }
    }
}
