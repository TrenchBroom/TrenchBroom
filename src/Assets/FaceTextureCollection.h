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

#ifndef __TrenchBroom__FaceTextureCollection__
#define __TrenchBroom__FaceTextureCollection__

#include "StringUtils.h"
#include "IO/Path.h"
#include "Assets/AssetTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class FaceTextureCollection {
        private:
            IO::Path m_path;
            FaceTextureList m_textures;
        public:
            FaceTextureCollection(const IO::Path& path, const FaceTextureList& textures);
            ~FaceTextureCollection();

            const IO::Path& path() const;
            const FaceTextureList& textures() const;
        private:
        };
        
    }
}


#endif /* defined(__TrenchBroom__FaceTextureCollection__) */
