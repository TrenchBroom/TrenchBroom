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

#include "Quake3ShaderTextureReader.h"

#include "Assets/Quake3Shader.h"
#include "Assets/Texture.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/MappedFile.h"

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderTextureReader::Quake3ShaderTextureReader(const NameStrategy& nameStrategy, const FileSystem& fs) :
        TextureReader(nameStrategy),
        m_fs(fs) {}

        Assets::Texture* Quake3ShaderTextureReader::doReadTexture(MappedFile::Ptr file) const {
            const auto* shaderFile = static_cast<ObjectFile<Assets::Quake3Shader>*>(file.get());
            const auto& shader = shaderFile->object();
            const auto& imagePath = shader.qerImagePath(Path("textures/__TB_empty.tga"));

            auto* texture = loadTextureImage(shader.texturePath(), imagePath);
            texture->setSurfaceParms(shader.surfaceParms());
            return texture;
        }

        Assets::Texture* Quake3ShaderTextureReader::loadTextureImage(const Path& shaderPath, const Path& imagePath) const {
            if (m_fs.fileExists(imagePath)) {
                FreeImageTextureReader imageReader(StaticNameStrategy(textureName(shaderPath)));
                return imageReader.readTexture(m_fs.openFile(imagePath));
            } else {
                return new Assets::Texture(textureName(shaderPath), 64, 64);
            }
        }
    }
}
