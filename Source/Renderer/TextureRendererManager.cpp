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

#include "TextureRendererManager.h"

#include "Model/Texture.h"
#include "Model/TextureManager.h"
#include "Renderer/TextureRenderer.h"

#include <cassert>
#include <exception>

namespace TrenchBroom {
    namespace Renderer {
        TextureRendererCollection::TextureRendererCollection(Model::TextureCollection& textureCollection, const Palette& palette) {
            typedef std::pair<TextureRendererMap::iterator, bool> InsertResult;

            Color averageColor;
            Model::TextureCollection::LoaderPtr loader = textureCollection.loader();

            const Model::TextureList& textures = textureCollection.textures();
            for (unsigned int i = 0; i < textures.size(); i++) {
                Model::Texture& texture = *textures[i];
                unsigned char* textureImage = loader->load(texture, palette, averageColor);
                if (textureImage != NULL) {
                    TextureRenderer* textureRenderer = new TextureRenderer(textureImage, averageColor, texture.width(), texture.height());
                    InsertResult result = m_textures.insert(TextureRendererEntry(&texture, textureRenderer));
                    assert(result.second);
                }
            }
        }
        
        TextureRenderer* TextureRendererCollection::renderer(Model::Texture& texture) const {
            TextureRendererMap::const_iterator it = m_textures.find(&texture);
            if (it == m_textures.end())
                return NULL;
            return it->second;
        }

        TextureRendererCollection::~TextureRendererCollection() {
            TextureRendererMap::iterator it, end;
            for (it = m_textures.begin(), end = m_textures.end(); it != end; ++it)
                delete it->second;
            m_textures.clear();
        }

        TextureRendererManager::TextureRendererManager(Model::TextureManager& textureManager) :
        m_textureManager(textureManager),
        m_dummyTexture(new TextureRenderer()),
        m_palette(NULL),
        m_valid(true) {}
        
        TextureRendererManager::~TextureRendererManager() {
            clear();
            delete m_dummyTexture;
            m_dummyTexture = NULL;
        }

        TextureRenderer& TextureRendererManager::renderer(Model::Texture* texture) {
            assert(m_palette != NULL);
            
            if (!m_valid) {
                clear();
                m_valid = true;
            }
            
            if (texture == NULL)
                return *m_dummyTexture;
            
            Model::TextureCollection& collection = texture->collection();
            TextureRendererCollection* rendererCollection = NULL;
            TextureRendererCollectionMap::iterator it = m_textureCollections.find(&collection);
            if (it == m_textureCollections.end()) {
                rendererCollection = new TextureRendererCollection(collection, *m_palette);
                m_textureCollections[&collection] = rendererCollection;
            } else {
                rendererCollection = it->second;
            }

            if (rendererCollection == NULL)
                return *m_dummyTexture;
            
            TextureRenderer* textureRenderer = rendererCollection->renderer(*texture);
            if (textureRenderer == NULL)
                return *m_dummyTexture;

            return *textureRenderer;
        }

        void TextureRendererManager::clear() {
            TextureRendererCollectionMap::iterator it, end;
            for (it = m_textureCollections.begin(), end = m_textureCollections.end(); it != end; ++it)
                delete it->second;
            m_textureCollections.clear();
        }
    }
}
