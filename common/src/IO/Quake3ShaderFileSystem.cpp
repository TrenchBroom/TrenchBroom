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
        Quake3ShaderFileSystem::Quake3ShaderFileSystem(std::unique_ptr<FileSystem> fs, const Path& texturePrefix, Logger* logger) :
        ImageFileSystemBase(std::move(fs), Path()),
        m_texturePrefix(texturePrefix),
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

            const auto scriptsPath = Path("scripts");
            if (next().directoryExists(scriptsPath)) {
                const auto paths = next().findItems(scriptsPath, FileExtensionMatcher("shader"));
                for (const auto& path : paths) {
                    // m_logger->debug() << "Loading shader " << path.asString();

                    const auto file = next().openFile(path);

                    Quake3ShaderParser parser(file->begin(), file->end());
                    VectorUtils::append(result, parser.parse());
                }
            }

            m_logger->info() << "Loaded " << result.size() << " shaders";
            return result;
        }

        void Quake3ShaderFileSystem::linkShaders(std::vector<Assets::Quake3Shader>& shaders) {
            const auto extensions = StringList { "tga", "png", "jpg", "jpeg" };
            const auto textures = next().findItemsRecursively(m_texturePrefix, FileExtensionMatcher(extensions));
            const auto skins = next().findItemsRecursively(Path("models"), FileExtensionMatcher(extensions));
            const auto allImages = VectorUtils::concatenate(textures, skins);

            m_logger->info() << "Linking shaders...";
            linkTextures(allImages, shaders);
            linkStandaloneShaders(shaders);
        }

        void Quake3ShaderFileSystem::linkTextures(const Path::List& textures, std::vector<Assets::Quake3Shader>& shaders) {
            m_logger->debug() << "Linking textures...";
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

                        auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shader, shaderPath);
                        m_root.addFile(shaderPath, std::make_unique<SimpleFile>(std::move(shaderFile)));

                        // Remove the shader so that we don't revisit it when linking standalone shaders.
                        shaders.erase(shaderIt);
                    } else {
                        // No matching shader found, generate one.
                        auto shader = Assets::Quake3Shader();
                        shader.shaderPath = shaderPath;
                        shader.editorImage = texture;

                        // m_logger->debug() << "Generating shader " << shaderPath << " -> " << shader.qerImagePath();

                        auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(std::move(shader), shaderPath);
                        m_root.addFile(shaderPath, std::make_unique<SimpleFile>(std::move(shaderFile)));
                    }
                }
            }
        }

        void Quake3ShaderFileSystem::linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders) {
            m_logger->debug() << "Linking standalone shaders...";
            for (auto& shader : shaders) {
                const auto& shaderPath = shader.shaderPath;
                auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shader, shaderPath);
                m_root.addFile(shaderPath, std::make_unique<SimpleFile>(std::move(shaderFile)));
            }
        }
    }
}
