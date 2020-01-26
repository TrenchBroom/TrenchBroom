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

#ifndef TRENCHBROOM_SKINLOADER_H
#define TRENCHBROOM_SKINLOADER_H

#include <memory>

namespace TrenchBroom {
    class Logger;
    
    namespace Assets {
        class Palette;
        class Texture;
    }

    namespace IO {
        class FileSystem;
        class Path;

        std::unique_ptr<Assets::Texture> loadSkin(const Path& path, const FileSystem& fs, Logger& logger);
        std::unique_ptr<Assets::Texture> loadSkin(const Path& path, const FileSystem& fs, Logger& logger, const Assets::Palette& palette);
        
        std::unique_ptr<Assets::Texture> loadShader(const Path& path, const FileSystem& fs, Logger& logger);
    }
}

#endif //TRENCHBROOM_SKINLOADER_H
