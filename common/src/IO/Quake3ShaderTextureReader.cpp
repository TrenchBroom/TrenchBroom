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
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "Renderer/GL.h"

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderTextureReader::Quake3ShaderTextureReader(const NameStrategy& nameStrategy, const FileSystem& fs) :
        TextureReader(nameStrategy),
        m_fs(fs) {}

        Assets::Texture* Quake3ShaderTextureReader::doReadTexture(std::shared_ptr<File> file) const {
            const auto* shaderFile = dynamic_cast<ObjectFile<Assets::Quake3Shader>*>(file.get());
            if (shaderFile == nullptr) {
                return nullptr;
            }

            const auto& shader = shaderFile->object();
            const auto texturePath = findTexturePath(shader);

            auto* texture = loadTextureImage(shader.shaderPath, texturePath);
            texture->setSurfaceParms(shader.surfaceParms);

            // Note that Quake 3 has a different understanding of front and back, so we need to invert them.
            switch (shader.culling) {
                case Assets::Quake3Shader::Culling::Front:
                    texture->setCulling(Assets::TextureCulling::CullBack);
                    break;
                case Assets::Quake3Shader::Culling::Back:
                    texture->setCulling(Assets::TextureCulling::CullFront);
                    break;
                case Assets::Quake3Shader::Culling::None:
                    texture->setCulling(Assets::TextureCulling::CullNone);
                    break;
            }

            if (!shader.stages.empty()) {
                const auto& stage = shader.stages.front();
                if (stage.blendFunc.enable()) {
                    texture->setBlendFunc(
                        glGetEnum(stage.blendFunc.srcFactor),
                        glGetEnum(stage.blendFunc.destFactor)
                    );
                }
            }

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

        Path Quake3ShaderTextureReader::findTexturePath(const Assets::Quake3Shader& shader) const {
            Path texturePath = findTexture(shader.editorImage);
            if (texturePath.isEmpty()) {
                texturePath = findTexture(shader.shaderPath);
            }
            if (texturePath.isEmpty()) {
                texturePath = findTexture(shader.lightImage);
            }
            if (texturePath.isEmpty()) {
                for (const auto& stage : shader.stages) {
                    texturePath = findTexture(stage.map);
                    if (!texturePath.isEmpty()) {
                        break;
                    }
                }
            }
            if (texturePath.isEmpty()) {
                texturePath = Path("textures/__TB_empty.tga");
            }
            return texturePath;
        }

        Path Quake3ShaderTextureReader::findTexture(const Path& texturePath) const {
            if (!texturePath.isEmpty() && (texturePath.extension().empty() || !m_fs.fileExists(texturePath))) {
                const auto candidates = m_fs.findItemsWithBaseName(texturePath, StringList { "tga", "png", "jpg", "jpeg"});
                if (!candidates.empty()) {
                    return candidates.front();
                } else {
                    return Path();
                }
            }
            // texture path is empty OR (the extension is not empty AND the file exists)
            return texturePath;
        }
    }
}
