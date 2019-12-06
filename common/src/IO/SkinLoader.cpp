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
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/File.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/WalTextureReader.h"

#include <kdl/string_format.h>

namespace TrenchBroom {
    namespace IO {
        Assets::Texture* loadSkin(std::shared_ptr<File> file) {
            return loadSkin(file, Assets::Palette());
        }

        Assets::Texture* loadSkin(std::shared_ptr<File> file, const Assets::Palette& palette) {
            try {
                ensure(file.get() != nullptr, "file is null");

                const Path path = file->path();
                const String extension = kdl::str_to_lower(path.extension());

                if (extension == "wal") {
                    WalTextureReader reader(TextureReader::PathSuffixNameStrategy(1, true), palette);
                    return reader.readTexture(file);
                } else {
                    FreeImageTextureReader reader(TextureReader::PathSuffixNameStrategy(1, true));
                    return reader.readTexture(file);
                }
            } catch (FileSystemException& e) {
                throw GameException("Could not load skin: " + String(e.what()));
            }
        }
    }
}
