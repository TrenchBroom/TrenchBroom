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
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType TextureCollectionCommand::Type = Command::freeType();

        TextureCollectionCommand::Ptr TextureCollectionCommand::add(View::MapDocumentPtr document, const IO::Path& path) {
            const IO::Path::List paths(1, path);
            return Ptr(new TextureCollectionCommand(document,
                                                    "Add Texture Collection",
                                                    AAdd,
                                                    paths));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::remove(View::MapDocumentPtr document, const IO::Path::List& paths) {
            return Ptr(new TextureCollectionCommand(document,
                                                    paths.size() == 1 ? "Remove Texture Collection" : "Remove Texture Collections",
                                                    ARemove,
                                                    paths));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveUp(View::MapDocumentPtr document, const IO::Path& path) {
            const IO::Path::List paths(1, path);
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collections Up",
                                                    AMoveUp,
                                                    paths));
        }
        
        TextureCollectionCommand::Ptr TextureCollectionCommand::moveDown(View::MapDocumentPtr document, const IO::Path& path) {
            const IO::Path::List paths(1, path);
            return Ptr(new TextureCollectionCommand(document,
                                                    "Move Texture Collections Up",
                                                    AMoveDown,
                                                    paths));
        }

        TextureCollectionCommand::TextureCollectionCommand(View::MapDocumentPtr document, const String& name, const Action action, const IO::Path::List& paths) :
        Command(Type, name, true, true),
        m_document(document),
        m_action(action),
        m_paths(paths) {
            switch (m_action) {
                case AAdd:
                case AMoveUp:
                case AMoveDown:
                    assert(paths.size() == 1);
                    break;
                case ARemove:
                    break;
            }
        }
        
        bool TextureCollectionCommand::doPerformDo() {
            m_previousCollections = m_document->externalTextureCollections();

            Model::Entity* worldspawn = m_document->worldspawn();
            m_document->objectWillChangeNotifier(worldspawn);
            
            switch (m_action) {
                case AAdd:
                    return addTextureCollections(m_paths);
                case ARemove:
                    return removeTextureCollections(m_paths);
                case AMoveUp:
                    return moveUp(m_paths.front());
                case AMoveDown:
                    return moveDown(m_paths.front());
            }

            m_document->objectDidChangeNotifier(worldspawn);
            m_document->textureCollectionsDidChangeNotifier();
        }
        
        bool TextureCollectionCommand::doPerformUndo() {
            Model::Entity* worldspawn = m_document->worldspawn();
            m_document->objectWillChangeNotifier(worldspawn);

            switch (m_action) {
                case AAdd:
                    return removeTextureCollections(m_paths);
                case ARemove:
                    return addTextureCollections(m_paths);
                case AMoveUp:
                    return moveDown(m_paths.front());
                case AMoveDown:
                    return moveUp(m_paths.front());
            }

            m_document->objectDidChangeNotifier(worldspawn);
            m_document->textureCollectionsDidChangeNotifier();
        }
        
        bool TextureCollectionCommand::addTextureCollections(const IO::Path::List& paths) {
            try {
                Assets::TextureManager& textureManager = m_document->textureManager();
                textureManager.addExternalTextureCollections(paths);
                return true;
            } catch (const Exception& e) {
                m_document->error(String(e.what()));
                return false;
            }
        }
        
        bool TextureCollectionCommand::removeTextureCollections(const IO::Path::List& paths) {
            Assets::TextureManager& textureManager = m_document->textureManager();
            textureManager.removeExternalTextureCollections(paths);
            return true;
        }
        
        bool TextureCollectionCommand::moveUp(const IO::Path& path) {
            Assets::TextureManager& textureManager = m_document->textureManager();
            textureManager.moveExternalTextureCollectionUp(path);
            return true;
        }
        
        bool TextureCollectionCommand::moveDown(const IO::Path& path) {
            Assets::TextureManager& textureManager = m_document->textureManager();
            textureManager.moveExternalTextureCollectionDown(path);
            return true;
        }
    }
}
