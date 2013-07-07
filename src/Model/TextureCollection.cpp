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

#include "TextureCollection.h"

namespace TrenchBroom {
    namespace Model {
        TextureCollection::Ptr TextureCollection::newTextureCollection(const IO::Path& path, const Texture::List& textures) {
            return Ptr(new TextureCollection(path, textures));
        }

        const IO::Path& TextureCollection::path() const {
            return m_path;
        }
        
        const Texture::List& TextureCollection::textures() const {
            return m_textures;
        }

        TextureCollection::TextureCollection(const IO::Path& path, const Texture::List& textures) :
        m_path(path),
        m_textures(textures) {}
    }
}
