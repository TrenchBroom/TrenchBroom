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

#ifndef __TrenchBroom__TextureRendererManager__
#define __TrenchBroom__TextureRendererManager__

#include "Model/Texture.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class Texture;
        class TextureCollection;
        class TextureManager;
    }
    
    namespace Renderer {
        class Palette;
        class TextureRenderer;
        
        class TextureRendererCollection {
        protected:
            typedef std::map<Model::Texture*, TextureRenderer*> TextureRendererMap;
            typedef std::pair<Model::Texture*, TextureRenderer*> TextureRendererEntry;
            
            TextureRendererMap m_textures;
        public:
            TextureRendererCollection(Model::TextureCollection& textureCollection, const Palette& palette);
            ~TextureRendererCollection();
            
            TextureRenderer* renderer(Model::Texture& texture) const;
        };
        
        class TextureRendererManager {
        protected:
            typedef std::map<Model::TextureCollection*, TextureRendererCollection*> TextureRendererCollectionMap;
            typedef std::pair<Model::TextureCollection*, TextureRendererCollection*> TextureRendererCollectionEntry;
            
            Model::TextureManager& m_textureManager;
            TextureRenderer* m_dummyTexture;
            Palette* m_palette;
            TextureRendererCollectionMap m_textureCollections;
            bool m_valid;
            
            void clear();
        public:
            TextureRendererManager(Model::TextureManager& textureManager);
            ~TextureRendererManager();
            
            inline void setPalette(Palette& palette) {
                if (&palette == m_palette)
                    return;
                m_palette = &palette;
                m_valid = false;
            }
            
            TextureRenderer& renderer(Model::Texture* texture);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureRendererManager__) */
