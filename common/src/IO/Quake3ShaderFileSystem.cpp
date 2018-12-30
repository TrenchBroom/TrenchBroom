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
        Quake3ShaderFileSystem::Quake3ShaderFileSystem(std::unique_ptr<FileSystem> fs, const Path& prefix, const StringList& extensions, Logger* logger) :
        ImageFileSystemBase(std::move(fs), Path()),
        m_prefix(prefix),
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

            const auto paths = next().findItems(Path("scripts"), FileExtensionMatcher("shader"));
            for (const auto& path : paths) {
                m_logger->debug() << "Loading shader " << path.asString();

                const auto file = next().openFile(path);

                Quake3ShaderParser parser(file->begin(), file->end());
                VectorUtils::append(result, parser.parse());
            }

            m_logger->info() << "Loaded " << result.size() << " shaders";

            return result;
        }

        void Quake3ShaderFileSystem::linkShaders(std::vector<Assets::Quake3Shader>& shaders) {
            const auto textures = next().findItemsRecursively(Path("textures"), FileExtensionMatcher(m_extensions));

            m_logger->info() << "Linking shaders...";
            linkTextures(textures, shaders);
            linkStandaloneShaders(shaders);
        }

        void Quake3ShaderFileSystem::linkTextures(const Path::List& textures, std::vector<Assets::Quake3Shader>& shaders) {
            m_logger->debug() << "Linking textures...";
            for (const auto& texture : textures) {
                const auto textureBasePath = texture.deleteExtension();

                // Only link a shader if it has not been linked yet.
                if (!fileExists(textureBasePath)) {
                    const auto shaderIt = std::find_if(std::begin(shaders), std::end(shaders), [&textureBasePath](const auto& shader){
                        return textureBasePath == shader.texturePath();
                    });

                    if (shaderIt != std::end(shaders)) {
                        // Found a shader, so we link to that.
                        const auto& shader = *shaderIt;
                        if (shader.hasQerImagePath()) {
                            // Link to the shader's editor image.
                            linkShaderToImage(texture, shader.qerImagePath(), shader);
                        } else {
                            // If it doesn't have an editor image, link to the texture path itself.
                            // This works because the link only considers files from chained file systems.
                            linkShaderToImage(texture, texture, shader);
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
                if (shader.hasQerImagePath()) {
                    linkShaderToImage(shader.texturePath(), shader.qerImagePath(), shader);
                } else {
                    linkShaderToMissingImage(shader.texturePath(), shader);
                }
            }
        }

        void Quake3ShaderFileSystem::linkShaderToImage(const Path& shaderPath, Path imagePath, const Assets::Quake3Shader& shader) {
            // Only link those shaders which have a valid path prefix, e.g. "/textures".
            if (shaderPath.hasPrefix(m_prefix, false)) {
                bool exists = next().fileExists(imagePath);
                if (!exists) {
                    // If the file does not exist, we try to find one with the same basename and a valid extension.
                    const auto candidates = next().findItemsWithBaseName(imagePath, m_extensions);
                    if (!candidates.empty()) {
                        const auto replacement = candidates.front(); // just use the first candidate
                        imagePath = replacement;
                        exists = true;
                    }
                }

                if (exists) {
                    doLinkShaderToImage(shaderPath, imagePath, shader);
                } else {
                    linkShaderToMissingImage(shaderPath, shader);
                }
            }
        }

        void Quake3ShaderFileSystem::linkShaderToMissingImage(const Path& shaderPath, const Assets::Quake3Shader& shader) {
            doLinkShaderToImage(shaderPath, Path("textures/__TB_empty.tga"), shader);
        }

        void Quake3ShaderFileSystem::doLinkShaderToImage(const Path& shaderPath, Path imagePath, const Assets::Quake3Shader& shader) {
            m_logger->debug() << "Linking shader " << shaderPath << " to image " << imagePath;

            auto link = std::make_unique<LinkFile>(next(), shaderPath, imagePath);
            link->setAttribute(Assets::Quake3Shader::SurfaceParms, shader.surfaceParms());
            m_root.addFile(shaderPath, std::move(link));
        }
    }
}
