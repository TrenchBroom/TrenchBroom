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

#include "Quake3ShaderFileSystem.h"

#include "Assets/Quake3Shader.h"
#include "IO/Quake3ShaderParser.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderFileSystem::Quake3ShaderFileSystem(const Path& path, const FileSystem& fs) :
        ImageFileSystemBase(path),
        m_fs(fs) {}

        void Quake3ShaderFileSystem::doReadDirectory() {
            const auto paths = m_fs.findItems(Path("scripts"), FileExtensionMatcher("shader"));
            for (const auto& path : paths) {
                const auto file = m_fs.openFile(path);
                processScript(file);
            }
        }

        void Quake3ShaderFileSystem::processScript(const MappedFile::Ptr& file) {
            Quake3ShaderParser parser(file->begin(), file->end());
            const auto shaders = parser.parse();
            for (const auto& shader : shaders) {
                processShader(shader);
            }
        }

        void Quake3ShaderFileSystem::processShader(const Assets::Quake3Shader& shader) {
            const auto& texturePath = shader.texturePath;
            const auto& imagePath = shader.qerImagePath;
            if (!imagePath.isEmpty()) {
                m_root.addFile(texturePath, std::make_unique<LinkFile>(texturePath, imagePath));
            } else {
                const auto paths = m_fs.findItems(texturePath, FileExtensionMatcher({"tga", "jpg", "jpeg", "png"}));
                if (!paths.empty()) {
                    m_root.addFile(texturePath, std::make_unique<LinkFile>(texturePath, paths.front()));
                } else {
                    // TODO: can't load this shader, we should link to a default image, but what?
                }
            }
        }
    }
}
