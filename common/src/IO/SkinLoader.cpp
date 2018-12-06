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

#include "Exceptions.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/IdMipTextureReader.h"
#include "IO/Path.h"
#include "IO/WalTextureReader.h"

namespace TrenchBroom {
    namespace IO {
        Assets::Texture* loadSkin(const IO::MappedFile::Ptr file) {
            return loadSkin(file, Assets::Palette());
        }

        Assets::Texture* loadSkin(const IO::MappedFile::Ptr file, const Assets::Palette& palette) {
            try {
                ensure(file.get() != nullptr, "file is null");

                const Path path = file->path();
                const String textureName = path.lastComponent().asString();
                const String extension = StringUtils::toLower(path.extension());

                if (extension == "wal") {
                    IO::WalTextureReader reader(IO::TextureReader::PathSuffixNameStrategy(1, true), palette);
                    return reader.readTexture(file);
                } else {
                    IO::FreeImageTextureReader reader(IO::TextureReader::PathSuffixNameStrategy(1, true));
                    return reader.readTexture(file);
                }
            } catch (FileSystemException& e) {
                throw GameException("Could not load skin: " + String(e.what()));
            }
        }
    }
}
