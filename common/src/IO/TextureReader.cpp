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

#include "TextureReader.h"

#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/ResourceUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        TextureReader::NameStrategy::NameStrategy() = default;

        TextureReader::NameStrategy::~NameStrategy() = default;

        TextureReader::NameStrategy* TextureReader::NameStrategy::clone() const {
            return doClone();
        }

        std::string TextureReader::NameStrategy::textureName(const std::string& textureName, const Path& path) const {
            return doGetTextureName(textureName, path);
        }

        TextureReader::TextureNameStrategy::TextureNameStrategy() = default;

        TextureReader::NameStrategy* TextureReader::TextureNameStrategy::doClone() const {
            return new TextureNameStrategy();
        }

        std::string TextureReader::TextureNameStrategy::doGetTextureName(const std::string& textureName, const Path& /* path */) const {
            return textureName;
        }

        TextureReader::PathSuffixNameStrategy::PathSuffixNameStrategy(const size_t suffixLength) :
        m_suffixLength(suffixLength) {}

        TextureReader::NameStrategy* TextureReader::PathSuffixNameStrategy::doClone() const {
            return new PathSuffixNameStrategy(m_suffixLength);
        }

        std::string TextureReader::PathSuffixNameStrategy::doGetTextureName(const std::string& /* textureName */, const Path& path) const {
            Path result = path.suffix(std::min(m_suffixLength, path.length())).deleteExtension();
            return result.asString("/");
        }

        TextureReader::StaticNameStrategy::StaticNameStrategy(const std::string& name) :
        m_name(name) {}

        TextureReader::NameStrategy* TextureReader::StaticNameStrategy::doClone() const {
            return new StaticNameStrategy(m_name);
        }

        std::string TextureReader::StaticNameStrategy::doGetTextureName(const std::string& /* textureName */, const Path& /* path */) const {
            return m_name;
        }

        TextureReader::TextureReader(const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger) :
        m_nameStrategy(nameStrategy.clone()),
        m_fs(fs),
        m_logger(logger) {}

        TextureReader::~TextureReader() {
            delete m_nameStrategy;
        }

        Assets::Texture* TextureReader::readTexture(std::shared_ptr<File> file) const {
            try {
                return doReadTexture(file);
            } catch (const AssetException& e) {
                m_logger.error() << "Could not read texture '" << file->path() << "': " << e.what();
                return loadDefaultTexture(m_fs, m_logger, textureName(file->path())).release();
            }
        }

        std::string TextureReader::textureName(const std::string& textureName, const Path& path) const {
            return m_nameStrategy->textureName(textureName, path);
        }

        std::string TextureReader::textureName(const Path& path) const {
            return m_nameStrategy->textureName(path.lastComponent().asString(), path);
        }

        bool TextureReader::checkTextureDimensions(const size_t width, const size_t height) {
            return width <= 8192 && height <= 8192;
        }

        size_t TextureReader::mipSize(const size_t width, const size_t height, const size_t mipLevel) {
            const auto size = Assets::sizeAtMipLevel(width, height, mipLevel);
            return size.x() * size.y();
        }
    }
}
