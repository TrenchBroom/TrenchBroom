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

#include "SkinLoader.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "Logger.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "IO/ResourceUtils.h"
#include "IO/WalTextureReader.h"

#include <kdl/string_format.h>

#include <string>

namespace TrenchBroom {
    namespace IO {
        Assets::Texture loadSkin(const Path& path, const FileSystem& fs, Logger& logger) {
            return loadSkin(path, fs, logger, Assets::Palette());
        }

        Assets::Texture loadSkin(const Path& path, const FileSystem& fs, Logger& logger, const Assets::Palette& palette) {
            const TextureReader::StaticNameStrategy nameStrategy(path.basename());

            try {
                const auto file = fs.openFile(path);
                const std::string extension = kdl::str_to_lower(path.extension());

                if (extension == "wal") {
                    WalTextureReader reader(nameStrategy, fs, logger, palette);
                    return reader.readTexture(file);
                } else {
                    FreeImageTextureReader reader(nameStrategy, fs, logger);
                    return reader.readTexture(file);
                }
            } catch (Exception& e) {
                logger.error() << "Could not load skin '" << path << "': " << e.what();
                return loadDefaultTexture(fs, logger, nameStrategy.textureName("", path));
            }
        }

        Assets::Texture loadShader(const Path& path, const FileSystem& fs, Logger& logger) {
            const TextureReader::PathSuffixNameStrategy nameStrategy(0u);
            
            if (!path.isEmpty()) {
                logger.debug() << "Loading shader '" << path << "'";
                try {
                    const auto file = fs.fileExists(path.deleteExtension()) ? fs.openFile(path.deleteExtension()) : fs.openFile(path);
                    
                    Quake3ShaderTextureReader reader(nameStrategy, fs, logger);
                    return reader.readTexture(file);
                } catch (const Exception& e) {
                    logger.error() << "Could not load shader '" << path << "': " << e.what();
                    // fall through to return the default texture
                }
            } else {
                logger.warn() << "Could not load shader: Path is empty";
            }
            const auto name = nameStrategy.textureName("", path);
            return loadDefaultTexture(fs, logger, name);
        }
    }
}
