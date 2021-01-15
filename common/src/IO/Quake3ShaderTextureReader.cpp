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
#include "IO/ResourceUtils.h"
#include "Renderer/GL.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderTextureReader::Quake3ShaderTextureReader(const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger) :
        TextureReader(nameStrategy, fs, logger) {}

        Assets::Texture Quake3ShaderTextureReader::doReadTexture(std::shared_ptr<File> file) const {
            const auto* shaderFile = dynamic_cast<ObjectFile<Assets::Quake3Shader>*>(file.get());
            if (shaderFile == nullptr) {
                throw AssetException("File is not a shader");
            }

            const auto& shader = shaderFile->object();
            const auto texturePath = findTexturePath(shader);
            if (texturePath.isEmpty()) {
                throw AssetException("Could not find texture path for shader '" + shader.shaderPath.asString() + "'");
            }

            auto texture = loadTextureImage(shader.shaderPath, texturePath);
            texture.setSurfaceParms(shader.surfaceParms);
            texture.setOpaque();

            // Note that Quake 3 has a different understanding of front and back, so we need to invert them.
            switch (shader.culling) {
                case Assets::Quake3Shader::Culling::Front:
                    texture.setCulling(Assets::TextureCulling::CullBack);
                    break;
                case Assets::Quake3Shader::Culling::Back:
                    texture.setCulling(Assets::TextureCulling::CullFront);
                    break;
                case Assets::Quake3Shader::Culling::None:
                    texture.setCulling(Assets::TextureCulling::CullNone);
                    break;
            }

            if (!shader.stages.empty()) {
                const auto& stage = shader.stages.front();
                if (stage.blendFunc.enable()) {
                    texture.setBlendFunc(
                        glGetEnum(stage.blendFunc.srcFactor),
                        glGetEnum(stage.blendFunc.destFactor)
                    );
                } else {
                    texture.disableBlend();
                }
            }

            return texture;
        }

        Assets::Texture Quake3ShaderTextureReader::loadTextureImage(const Path& shaderPath, const Path& imagePath) const {
            const auto name = textureName(shaderPath);
            if (!m_fs.fileExists(imagePath)) {
                throw AssetException("Image file '" + imagePath.asString() + "' does not exist");
            }

            FreeImageTextureReader imageReader(StaticNameStrategy(name), m_fs, m_logger);
            return imageReader.readTexture(m_fs.openFile(imagePath));
        }

        Path Quake3ShaderTextureReader::findTexturePath(const Assets::Quake3Shader& shader) const {
            Path texturePath = findTexture(shader.editorImage);
            if (texturePath.isEmpty()) {
                texturePath = findTexture(shader.diffuseImage);
            }
            if (texturePath.isEmpty()) {
                for (const auto& stage : shader.stages) {
                    if(stage.lighting == Assets::Quake3ShaderStage::StageLighting::Diffuse) {
                        texturePath = findTexture(stage.map);
                        if (!texturePath.isEmpty()) {
                            break;
                        }
                    }
                }
            }
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
            return texturePath;
        }

        Path Quake3ShaderTextureReader::findTexture(const Path& texturePath) const {
            if (!texturePath.isEmpty() && (texturePath.extension().empty() || !m_fs.fileExists(texturePath))) {
                const auto candidates = m_fs.findItemsWithBaseName(texturePath, std::vector<std::string> { "tga", "png", "jpg", "jpeg"});
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
