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

#include "Logger.h"
#include "Assets/Quake3Shader.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Quake3ShaderParser.h"
#include "IO/SimpleParserStatus.h"

#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderFileSystem::Quake3ShaderFileSystem(std::shared_ptr<FileSystem> fs, Path shaderSearchPath, std::vector<Path> textureSearchPaths, Logger& logger) :
        ImageFileSystemBase(std::move(fs), Path()),
        m_shaderSearchPath(std::move(shaderSearchPath)),
        m_textureSearchPaths(std::move(textureSearchPaths)),
        m_logger(logger) {
            initialize();
        }

        void Quake3ShaderFileSystem::doReadDirectory() {
            if (hasNext()) {
                auto shaders = loadShaders();
                linkShaders(shaders);
            }
        }

        std::vector<Assets::Quake3Shader> Quake3ShaderFileSystem::loadShaders() const {
            auto result = std::vector<Assets::Quake3Shader>();

            if (next().directoryExists(m_shaderSearchPath)) {
                const auto paths = next().findItems(m_shaderSearchPath, FileExtensionMatcher("shader"));
                for (const auto& path : paths) {
                    const auto file = next().openFile(path);
                    auto bufferedReader = file->reader().buffer();

                    try {
                        Quake3ShaderParser parser(bufferedReader.stringView());
                        SimpleParserStatus status(m_logger, file->path().asString());
                        kdl::vec_append(result, parser.parse(status));
                    } catch (const ParserException& e) {
                        m_logger.warn() << "Skipping malformed shader file " << path << ": " << e.what();
                    }
                }
            }

            m_logger.info() << "Loaded " << result.size() << " shaders";
            return result;
        }

        void Quake3ShaderFileSystem::linkShaders(std::vector<Assets::Quake3Shader>& shaders) {
            const auto extensions = std::vector<std::string> { "tga", "png", "jpg", "jpeg" };

            auto allImages = std::vector<Path>();
            for (const auto& path : m_textureSearchPaths) {
                if (next().directoryExists(path)) {
                    kdl::vec_append(allImages, next().findItemsRecursively(path, FileExtensionMatcher(extensions)));
                }
            }

            m_logger.info() << "Linking shaders...";
            linkTextures(allImages, shaders);
            linkStandaloneShaders(shaders);
        }

        void Quake3ShaderFileSystem::linkTextures(const std::vector<Path>& textures, std::vector<Assets::Quake3Shader>& shaders) {
            m_logger.debug() << "Linking textures...";
            for (const auto& texture : textures) {
                const auto shaderPath = texture.deleteExtension();

                // Only link a shader if it has not been linked yet.
                if (!fileExists(shaderPath)) {
                    const auto shaderIt = std::find_if(std::begin(shaders), std::end(shaders), [&shaderPath](const auto& shader){
                        return shaderPath == shader.shaderPath;
                    });

                    if (shaderIt != std::end(shaders)) {
                        // Found a matching shader.
                        auto& shader = *shaderIt;

                        auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shaderPath, shader);
                        m_root.addFile(shaderPath, shaderFile);

                        // Remove the shader so that we don't revisit it when linking standalone shaders.
                        shaders.erase(shaderIt);
                    } else {
                        // No matching shader found, generate one.
                        auto shader = Assets::Quake3Shader();
                        shader.shaderPath = shaderPath;
                        shader.editorImage = texture;

                        auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shaderPath, std::move(shader));
                        m_root.addFile(shaderPath, std::move(shaderFile));
                    }
                }
            }
        }

        void Quake3ShaderFileSystem::linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders) {
            m_logger.debug() << "Linking standalone shaders...";
            for (auto& shader : shaders) {
                const auto& shaderPath = shader.shaderPath;
                auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shaderPath, shader);
                m_root.addFile(shaderPath, std::move(shaderFile));
            }
        }
    }
}
