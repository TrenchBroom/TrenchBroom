/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "TextureReader.h"

#include "IO/FileSystem.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        TextureReader::NameStrategy::NameStrategy() {}

        TextureReader::NameStrategy::~NameStrategy() {}
        
        TextureReader::NameStrategy* TextureReader::NameStrategy::clone() const {
            return doClone();
        }

        String TextureReader::NameStrategy::textureName(const String& textureName, const Path& path) const {
            return doGetTextureName(textureName, path);
        }

        TextureReader::TextureNameStrategy::TextureNameStrategy() {}

        TextureReader::NameStrategy* TextureReader::TextureNameStrategy::doClone() const {
            return new TextureNameStrategy();
        }
        
        String TextureReader::TextureNameStrategy::doGetTextureName(const String& textureName, const Path& path) const {
            return textureName;
        }
        
        TextureReader::PathSuffixNameStrategy::PathSuffixNameStrategy(const size_t suffixLength, const bool deleteExtension) :
        m_suffixLength(suffixLength),
        m_deleteExtension(deleteExtension) {}

        TextureReader::NameStrategy* TextureReader::PathSuffixNameStrategy::doClone() const {
            return new PathSuffixNameStrategy(m_suffixLength, m_deleteExtension);
        }
        
        String TextureReader::PathSuffixNameStrategy::doGetTextureName(const String& textureName, const Path& path) const {
            Path result = path.suffix(std::min(m_suffixLength, path.length()));
            if (m_deleteExtension)
                result = result.deleteExtension();
            return result.asString('/');
        }

        TextureReader::TextureReader(const NameStrategy& nameStrategy) :
        m_nameStrategy(nameStrategy.clone()) {}

        TextureReader::~TextureReader() {
            delete m_nameStrategy;
        }
        
        Assets::Texture* TextureReader::readTexture(MappedFile::Ptr file) const {
            return readTexture(file->begin(), file->end(), file->path());
        }

        Assets::Texture* TextureReader::readTexture(const char* const begin, const char* const end, const Path& path) const {
            return doReadTexture(begin, end, path);
        }

        String TextureReader::textureName(const String& textureName, const Path& path) const {
            return m_nameStrategy->textureName(textureName, path);
        }

        size_t TextureReader::mipSize(const size_t width, const size_t height, const size_t mipLevel) {
            const size_t divisor = 1 << mipLevel;
            return (width * height) / (divisor * divisor);
        }
    }
}
