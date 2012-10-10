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
#include "Model/MapDocument.h"
#include "Model/TextureManager.h"

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
            while (!collections.empty()) delete collections.back(), collections.pop_back();
        }

        TextureCollectionCommand::TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const String& path) :
        DocumentCommand(type, document, true, name) {
            m_paths.push_back(path);
        }

        TextureCollectionCommand::TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const IndexList& indices) :
        DocumentCommand(type, document, true, name),
        m_indices(indices) {}


        bool TextureCollectionCommand::performDo() {
            if (type() == AddTextureCollection) {
                m_indices.clear();
                addTextureCollectionsByPaths();
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
                document().UpdateAllViews(NULL, this);
                return true;
            }
            
            return false;
        }
        
        bool TextureCollectionCommand::performUndo() {
            if (type() == AddTextureCollection) {
                removeTextureCollectionsByPaths();
                document().UpdateAllViews(NULL, this);
                return true;
            } else if (type() == RemoveTextureCollection) {
                addTextureCollectionsByPaths();
                Model::TextureManager& textureManager = document().textureManager();
                Model::Texture* mruTexture = textureManager.texture(m_mruTextureName);
                document().setMruTexture(mruTexture);
                document().UpdateAllViews(NULL, this);
                return true;
            }
            
            return false;
        }

        TextureCollectionCommand* TextureCollectionCommand::addTextureWad(Model::MapDocument& document, const String& path) {
            return new TextureCollectionCommand(AddTextureCollection, document, "Add texture wad", path);
        }
        
        TextureCollectionCommand* TextureCollectionCommand::removeTextureWads(Model::MapDocument& document, const IndexList& indices) {
            return new TextureCollectionCommand(RemoveTextureCollection, document, indices.size() == 1 ? "Remove texture wad" : "Remove texture wads", indices);
        }
    }
}