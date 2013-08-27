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

#include "FaceTextureCollection.h"

#include "CollectionUtils.h"
#include "Assets/FaceTexture.h"

namespace TrenchBroom {
    namespace Assets {
        FaceTextureCollection::FaceTextureCollection(const IO::Path& path, const FaceTextureList& textures) :
        m_path(path),
        m_textures(textures) {}

        FaceTextureCollection::~FaceTextureCollection() {
            VectorUtils::clearAndDelete(m_textures);
        }

        const IO::Path& FaceTextureCollection::path() const {
            return m_path;
        }
        
        const FaceTextureList& FaceTextureCollection::textures() const {
            return m_textures;
        }

    }
}
