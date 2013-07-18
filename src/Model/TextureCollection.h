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
#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class TextureCollection {
        private:
            IO::Path m_path;
            TextureList m_textures;
        public:
            static TextureCollectionPtr newTextureCollection(const IO::Path& path, const TextureList& textures);
            
            const IO::Path& path() const;
            const TextureList& textures() const;
        private:
            TextureCollection(const IO::Path& path, const TextureList& textures);
        };
        
    }
}


#endif /* defined(__TrenchBroom__TextureCollection__) */
