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

#pragma once

#include "IO/FileSystem.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
        class Quake3ShaderFileSystem;
    }

    namespace Model {
        struct GameConfig;

        class GameFileSystem : public IO::FileSystem {
        private:
            IO::Quake3ShaderFileSystem* m_shaderFS;
        public:
            GameFileSystem();
            void initialize(const GameConfig& config, const IO::Path& gamePath, const std::vector<IO::Path>& additionalSearchPaths, Logger& logger);
            void reloadShaders();
        private:
            void addDefaultAssetPaths(const GameConfig& config, Logger& logger);
            void addGameFileSystems(const GameConfig& config, const IO::Path& gamePath, const std::vector<IO::Path>& additionalSearchPaths, Logger& logger);
            void addShaderFileSystem(const GameConfig& config, Logger& logger);
            void addFileSystemPath(const IO::Path& path, Logger& logger);
            void addFileSystemPackages(const GameConfig& config, const IO::Path& searchPath, Logger& logger);
        private:
            bool doDirectoryExists(const IO::Path& path) const override;
            bool doFileExists(const IO::Path& path) const override;
            std::vector<IO::Path> doGetDirectoryContents(const IO::Path& path) const override;
            std::shared_ptr<IO::File> doOpenFile(const IO::Path& path) const override;
        };
    }
}

