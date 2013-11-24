/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

        TextureCollectionCommand::Ptr TextureCollectionCommand::add(View::MapDocumentPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Add Texture Collection",
                                                    AAdd,
                                                    StringList(1, name)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::remove(View::MapDocumentPtr document, const StringList& names) {
            return Ptr(new TextureCollectionCommand(document,
                                                    names.size() == 1 ? "Remove Texture Collection" : "Remove Texture Collections",
                                                    ARemove,
                                                    names));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveUp(View::MapDocumentPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collection Up",
                                                    AMoveUp,
                                                    StringList(1, name)));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveDown(View::MapDocumentPtr document, const String& name) {
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collection Down",
                                                    AMoveDown,
                                                    StringList(1, name)));
        }

        TextureCollectionCommand::TextureCollectionCommand(View::MapDocumentPtr document, const String& name, const Action action, const StringList& names) :
        Command(Type, name, true, true),
        m_document(document),
        m_action(action),
        m_names(names) {
            switch (m_action) {
                case AAdd:
                case AMoveUp:
                case AMoveDown:
                    assert(m_names.size() == 1);
                    break;
                case ARemove:
                    break;
            }
        }
        
        bool TextureCollectionCommand::doPerformDo() {
            Model::Entity* worldspawn = m_document->worldspawn();
            
            switch (m_action) {
                case AAdd:
                    m_document->addExternalTextureCollections(m_names);
                    break;
                case ARemove:
                    m_document->removeExternalTextureCollections(m_names);
                    break;
                case AMoveUp:
                    m_document->moveExternalTextureCollectionUp(m_names.front());
                    break;
                case AMoveDown:
                    m_document->moveExternalTextureCollectionDown(m_names.front());
                    break;
            }

            m_document->objectWillChangeNotifier(worldspawn);
            m_document->updateExternalTextureCollectionProperty();
            m_document->objectDidChangeNotifier(worldspawn);
            m_document->textureCollectionsDidChangeNotifier();
            return true;
        }
        
        bool TextureCollectionCommand::doPerformUndo() {
            Model::Entity* worldspawn = m_document->worldspawn();

            switch (m_action) {
                case AAdd:
                    m_document->removeExternalTextureCollections(m_names);
                    break;
                case ARemove:
                    m_document->addExternalTextureCollections(m_names);
                    break;
                case AMoveUp:
                    m_document->moveExternalTextureCollectionDown(m_names.front());
                    break;
                case AMoveDown:
                    m_document->moveExternalTextureCollectionUp(m_names.front());
                    break;
            }

            m_document->objectWillChangeNotifier(worldspawn);
            m_document->updateExternalTextureCollectionProperty();
            m_document->objectDidChangeNotifier(worldspawn);
            m_document->textureCollectionsDidChangeNotifier();
            return true;
        }
    }
}
