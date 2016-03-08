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

#ifndef TrenchBroom_TextureCollection
#define TrenchBroom_TextureCollection

#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Renderer/GL.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureCollection {
        private:
            typedef std::vector<GLuint> TextureIdList;
            
            bool m_loaded;
            String m_name;
            TextureList m_textures;
            TextureIdList m_textureIds;
        public:
            TextureCollection(const String& name);
            TextureCollection(const String& name, const TextureList& textures);
            virtual ~TextureCollection();

            bool loaded() const;
            const String& name() const;
            const TextureList& textures() const;

            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);
        };
    }
}


#endif /* defined(TrenchBroom_TextureCollection) */
