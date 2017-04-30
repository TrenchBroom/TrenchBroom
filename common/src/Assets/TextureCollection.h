/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_TextureCollection
#define TrenchBroom_TextureCollection

#include "Notifier.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Renderer/GL.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureCollection {
        private:
            typedef std::vector<GLuint> TextureIdList;
            
            bool m_loaded;
            IO::Path m_path;
            TextureList m_textures;
            
            size_t m_usageCount;
            
            TextureIdList m_textureIds;
            
            friend class Texture;
        public:
            Notifier0 usageCountDidChange;
        public:
            TextureCollection();
            explicit TextureCollection(const TextureList& textures);
            explicit TextureCollection(const IO::Path& path);
            TextureCollection(const IO::Path& path, const TextureList& textures);
            virtual ~TextureCollection();

            void addTextures(const TextureList& textures);
            void addTexture(Texture* texture);
            
            bool loaded() const;
            const IO::Path& path() const;
            String name() const;
            const TextureList& textures() const;

            size_t usageCount() const;
            
            bool prepared() const;
            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);
        private:
            void incUsageCount();
            void decUsageCount();
        };
    }
}


#endif /* defined(TrenchBroom_TextureCollection) */
