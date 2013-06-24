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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__TextureCollection__
#define __TrenchBroom__TextureCollection__

#include "StringUtils.h"
#include "Model/Texture.h"
#include "SharedPointer.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {

        class TextureCollection {
        public:
            typedef std::tr1::shared_ptr<TextureCollection> Ptr;
            typedef std::vector<Ptr> List;
        private:
            String m_name;
            TextureList m_textures;
        public:
            static Ptr newTextureCollection(const String& name, const TextureList& textures);
            
            inline const TextureList& textures() const {
                return m_textures;
            }
        private:
            TextureCollection(const String& name, const TextureList& textures);
        };
        
    }
}


#endif /* defined(__TrenchBroom__TextureCollection__) */
