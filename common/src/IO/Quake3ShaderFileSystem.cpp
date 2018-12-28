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

#include "CollectionUtils.h"
#include "Assets/Quake3Shader.h"
#include "IO/Quake3ShaderParser.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderFileSystem::Quake3ShaderFileSystem(const FileSystem& fs, const StringList& extensions, Logger* logger) :
        ImageFileSystemBase(Path()),
        m_fs(fs),
        m_extensions(extensions),
        m_logger(logger) {
            initialize();
        }

        void Quake3ShaderFileSystem::doReadDirectory() {
            auto shaders = loadShaders();
            linkShaders(shaders);
        }

        std::vector<Assets::Quake3Shader> Quake3ShaderFileSystem::loadShaders() const {
            auto result = std::vector<Assets::Quake3Shader>();

            const auto paths = m_fs.findItems(Path("scripts"), FileExtensionMatcher("shader"));
            for (const auto& path : paths) {
                m_logger->debug() << "Loading shader " << path.asString();

                const auto file = m_fs.openFile(path);

                Quake3ShaderParser parser(file->begin(), file->end());
                VectorUtils::append(result, parser.parse());
            }

            m_logger->info() << "Loaded " << result.size() << " shaders";

            return result;
        }

        void Quake3ShaderFileSystem::linkShaders(std::vector<Assets::Quake3Shader>& shaders) {
            const auto textures = m_fs.findItemsRecursively(Path("textures"), FileExtensionMatcher(m_extensions));

            m_logger->info() << "Linking shaders...";
            linkTextures(textures, shaders);
            linkStandaloneShaders(shaders);
        }

        void Quake3ShaderFileSystem::linkTextures(const Path::List& textures, std::vector<Assets::Quake3Shader>& shaders) {
            m_logger->debug() << "Linking textures...";
            for (const auto& texture : textures) {
                const auto textureBasePath = texture.deleteExtension();

                // Only link a shader if it has not been linked yet.
                if (!fileExists(texture)) {
                    const auto shaderIt = std::find_if(std::begin(shaders), std::end(shaders), [&textureBasePath](const auto& shader){
                        return textureBasePath == shader.texturePath;
                    });

                    if (shaderIt != std::end(shaders)) {
                        // Found a shader. If it has an editor image, we link to that, but only if the editor image is
                        // different from the texture image.
                        const auto& shader = *shaderIt;
                        if (!shader.qerImagePath.isEmpty() && texture != shader.qerImagePath) {
                            linkShaderToImage(texture, shader.qerImagePath);
                        }

                        // Remove the shader so that we don't revisit it when linking standalone shaders.
                        shaders.erase(shaderIt);
                    }
                }
            }
        }

        void Quake3ShaderFileSystem::linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders) {
            m_logger->debug() << "Linking standalone shaders...";
            for (const auto& shader : shaders) {
                if (!shader.qerImagePath.isEmpty()) {
                    linkShaderToImage(shader.texturePath, shader.qerImagePath);
                } else {
                    linkShaderToImage(shader.texturePath, Path("textures/__TB_empty.tga"));
                }
            }
        }

        void Quake3ShaderFileSystem::linkShaderToImage(const Path& shaderPath, Path imagePath) {
            if (!m_fs.fileExists(imagePath)) {
                // If the file does not exist, we try to find one with the same basename and a valid extension.
                const auto candidates = m_fs.findItemsWithBaseName(imagePath, m_extensions);
                if (!candidates.empty()) {
                    const auto replacement = candidates.front(); // just use the first candidate
                    imagePath = replacement;
                }
            }
            // Don't link a file to itself.
            if (shaderPath != imagePath) {
                m_logger->debug() << "Linking shader: " << shaderPath << " -> " << imagePath;
                m_root.addFile(shaderPath, std::make_unique<LinkFile>(shaderPath, imagePath));
            }
        }
    }
}
