/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextureCollectionCommand.h"

#include "IO/Wad.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/TextureManager.h"
#include "Utility/List.h"

namespace TrenchBroom {
    namespace Controller {
        void TextureCollectionCommand::addTextureCollectionsByPaths() {
            for (unsigned int i = 0; i < m_paths.size(); i++) {
                if (i < m_indices.size())
                    document().loadTextureWad(m_paths[i], m_indices[i]);
                else
                    document().loadTextureWad(m_paths[i]);
            }
            document().updateAfterTextureManagerChanged();
        }

        void TextureCollectionCommand::removeTextureCollectionsByPaths() {
            m_indices.clear();
            Model::TextureManager& textureManager = document().textureManager();
            Model::TextureCollectionList collections;
            for (unsigned int i = 0; i < m_paths.size(); i++) {
                size_t index = textureManager.indexOfTextureCollection(m_paths[i]);
                Model::TextureCollection* collection = textureManager.removeCollection(index);
                if (collection != NULL) {
                    collections.push_back(collection);
                    m_indices.push_back(index);
                }
            }
            
            document().updateAfterTextureManagerChanged();
            Utility::deleteAll(collections);
        }
        
        size_t TextureCollectionCommand::moveTextureCollection(bool up) {
            assert(m_indices.size() == 1);
            size_t index = m_indices.front();
            assert(!up || index > 0);
            
            Model::TextureManager& textureManager = document().textureManager();
            assert(index < textureManager.collections().size());
            assert(up || index < textureManager.collections().size() - 1);
            
            Model::TextureCollection* collection = textureManager.removeCollection(index);
            
            size_t newIndex = index;
            if (up)
                newIndex -= 1;
            else
                newIndex += 1;
            
            textureManager.addCollection(collection, newIndex);
            document().updateAfterTextureManagerChanged();
            return newIndex;
        }

        void TextureCollectionCommand::updateWadKey() {
            Model::TextureManager& textureManager = document().textureManager();
            Model::Entity& worldspawn = *document().worldspawn(true);
            worldspawn.setProperty(Model::Entity::WadKey, textureManager.wadProperty());
        }

        TextureCollectionCommand::TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const String& path) :
        DocumentCommand(type, document, true, name, true) {
            m_paths.push_back(path);
        }

        TextureCollectionCommand::TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const IndexList& indices) :
        DocumentCommand(type, document, true, name, true),
        m_indices(indices) {}


        bool TextureCollectionCommand::performDo() {
            if (type() == AddTextureCollection) {
                m_indices.clear();
                addTextureCollectionsByPaths();
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == RemoveTextureCollection) {
                Model::Texture* mruTexture = document().mruTexture();
                m_mruTextureName = mruTexture != NULL ? mruTexture->name() : "";
                m_paths.clear();

                Model::TextureManager& textureManager = document().textureManager();
                for (unsigned int i = 0; i < m_indices.size(); i++) {
                    Model::TextureCollection* collection = textureManager.collections()[m_indices[i]];
                    m_paths.push_back(collection->name());
                }
                removeTextureCollectionsByPaths();
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == MoveTextureCollectionUp) {
                size_t newIndex = moveTextureCollection(true);
                m_indices.clear();
                m_indices.push_back(newIndex);
                
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == MoveTextureCollectionDown) {
                size_t newIndex = moveTextureCollection(false);
                m_indices.clear();
                m_indices.push_back(newIndex);
                
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            }
            
            return false;
        }
        
        bool TextureCollectionCommand::performUndo() {
            if (type() == AddTextureCollection) {
                removeTextureCollectionsByPaths();
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == RemoveTextureCollection) {
                addTextureCollectionsByPaths();
                Model::TextureManager& textureManager = document().textureManager();
                Model::Texture* mruTexture = textureManager.texture(m_mruTextureName);
                document().setMruTexture(mruTexture);
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == MoveTextureCollectionUp) {
                size_t newIndex = moveTextureCollection(false);
                m_indices.clear();
                m_indices.push_back(newIndex);
                
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == MoveTextureCollectionDown) {
                size_t newIndex = moveTextureCollection(true);
                m_indices.clear();
                m_indices.push_back(newIndex);
                
                updateWadKey();
                document().UpdateAllViews(NULL, this);
                return true;
            }
            
            return false;
        }

        TextureCollectionCommand* TextureCollectionCommand::addTextureWad(Model::MapDocument& document, const String& path) {
            return new TextureCollectionCommand(AddTextureCollection, document, "Add texture wad", path);
        }
        
        TextureCollectionCommand* TextureCollectionCommand::removeTextureCollections(Model::MapDocument& document, const IndexList& indices) {
            return new TextureCollectionCommand(RemoveTextureCollection, document, indices.size() == 1 ? "Remove texture wad" : "Remove texture wads", indices);
        }

        TextureCollectionCommand* TextureCollectionCommand::moveTextureCollectionUp(Model::MapDocument& document, size_t index) {
            IndexList indices;
            indices.push_back(index);
            return new TextureCollectionCommand(MoveTextureCollectionUp, document, "Move texture wad up", indices);
        }
        
        TextureCollectionCommand* TextureCollectionCommand::moveTextureCollectionDown(Model::MapDocument& document, size_t index) {
            IndexList indices;
            indices.push_back(index);
            return new TextureCollectionCommand(MoveTextureCollectionDown, document, "Move texture wad up", indices);
        }
    }
}
