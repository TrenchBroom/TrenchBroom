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

#include "TextureLoader.h"

#include "Assets/Palette.h"
#include "Assets/TextureManager.h"
#include "EL/Interpolator.h"
#include "IO/IdMipTextureReader.h"
#include "IO/IdWalTextureReader.h"
#include "IO/Path.h"
#include "IO/TextureCollectionLoader.h"
#include "Model/GameConfig.h"

namespace TrenchBroom {
    namespace IO {
        TextureLoader::TextureLoader(const EL::VariableStore& variables, const FileSystem& gameFS, const IO::Path::List& fileSearchPaths, const Model::GameConfig::TextureConfig& textureConfig) :
        m_variables(variables.clone()),
        m_gameFS(gameFS),
        m_fileSearchPaths(fileSearchPaths),
        m_textureExtension(getTextureExtension(textureConfig)),
        m_textureReader(createTextureReader(textureConfig)),
        m_textureCollectionLoader(createTextureCollectionLoader(textureConfig)) {
            ensure(m_textureReader != NULL, "textureReader is null");
            ensure(m_textureCollectionLoader != NULL, "textureCollectionLoader is null");
        }
        
        TextureLoader::~TextureLoader() {
            delete m_textureCollectionLoader;
            delete m_textureReader;
            delete m_variables;
        }
        
        String TextureLoader::getTextureExtension(const Model::GameConfig::TextureConfig& textureConfig) const {
            return textureConfig.format.extension;
        }
        
        TextureReader* TextureLoader::createTextureReader(const Model::GameConfig::TextureConfig& textureConfig) const {
            if (textureConfig.format.format == "idmip") {
                TextureReader::PathSuffixNameStrategy nameStrategy(1, true);
                return new IdMipTextureReader(nameStrategy, loadPalette(textureConfig));
            } else if (textureConfig.format.format == "idwal") {
                TextureReader::PathSuffixNameStrategy nameStrategy(2, true);
                return new IdWalTextureReader(nameStrategy, loadPalette(textureConfig));
            } else {
                throw GameException("Unknown texture format '" + textureConfig.format.format + "'");
            }
        }
        
        Assets::Palette TextureLoader::loadPalette(const Model::GameConfig::TextureConfig& textureConfig) const {
            const String pathSpec = textureConfig.palette.asString();
            const String pathStr = EL::interpolate(pathSpec, EL::EvaluationContext(*m_variables));
            const Path path(pathStr);
            return Assets::Palette::loadFile(m_gameFS, path);
        }

        TextureCollectionLoader* TextureLoader::createTextureCollectionLoader(const Model::GameConfig::TextureConfig& textureConfig) const {
            using Model::GameConfig;
            switch (textureConfig.package.type) {
                case GameConfig::TexturePackageConfig::PT_File:
                    return new FileTextureCollectionLoader(m_fileSearchPaths);
                case GameConfig::TexturePackageConfig::PT_Directory:
                    return new DirectoryTextureCollectionLoader(m_gameFS);
                case GameConfig::TexturePackageConfig::PT_Unset:
                    throw GameException("Texture package format is not set");
				switchDefault()
            }
        }

        Assets::TextureCollection* TextureLoader::loadTextureCollection(const Path& path) {
            return m_textureCollectionLoader->loadTextureCollection(path, m_textureExtension, *m_textureReader);
        }

        void TextureLoader::loadTextures(const Path::List& paths, Assets::TextureManager& textureManager) {
            textureManager.setTextureCollections(paths, *this);
        }
    }
}
