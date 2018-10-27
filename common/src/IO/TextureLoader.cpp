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
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/HlMipTextureReader.h"
#include "IO/IdMipTextureReader.h"
#include "IO/WalTextureReader.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"

namespace TrenchBroom {
    namespace IO {
        TextureLoader::TextureLoader(const EL::VariableStore& variables, const FileSystem& gameFS, const IO::Path::List& fileSearchPaths, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger) :
        m_textureExtensions(getTextureExtensions(textureConfig)),
        m_textureReader(createTextureReader(variables, gameFS, textureConfig, logger)),
        m_textureCollectionLoader(createTextureCollectionLoader(gameFS, fileSearchPaths, textureConfig)) {
            ensure(m_textureReader != nullptr, "textureReader is null");
            ensure(m_textureCollectionLoader != nullptr, "textureCollectionLoader is null");
        }
        
        StringList TextureLoader::getTextureExtensions(const Model::GameConfig::TextureConfig& textureConfig) {
            return textureConfig.format.extensions;
        }

        TextureLoader::ReaderPtr TextureLoader::createTextureReader(const EL::VariableStore& variables, const FileSystem& gameFS, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger) {
            if (textureConfig.format.format == "idmip") {
                TextureReader::PathSuffixNameStrategy nameStrategy(1, true);
                return std::make_unique<IdMipTextureReader>(nameStrategy, loadPalette(variables, gameFS, textureConfig, logger));
            } else if (textureConfig.format.format == "hlmip") {
                TextureReader::PathSuffixNameStrategy nameStrategy(1, true);
                return std::make_unique<HlMipTextureReader>(nameStrategy);
            } else if (textureConfig.format.format == "wal") {
                TextureReader::PathSuffixNameStrategy nameStrategy(2, true);
                return std::make_unique<WalTextureReader>(nameStrategy, loadPalette(variables, gameFS, textureConfig, logger));
            } else if (textureConfig.format.format == "image") {
                TextureReader::PathSuffixNameStrategy nameStrategy(2, true);
                return std::make_unique<FreeImageTextureReader>(nameStrategy, 4);
            } else {
                throw GameException("Unknown texture format '" + textureConfig.format.format + "'");
            }
        }
        
        Assets::Palette TextureLoader::loadPalette(const EL::VariableStore& variables, const FileSystem& gameFS, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger) {
            try {
                const Path path = findPalette(variables, gameFS, textureConfig, logger);
                logger->info("Loading palette file " + path.asString());
                return Assets::Palette::loadFile(gameFS, path);
            } catch (const Exception& e) {
                if (logger != nullptr) {
                    logger->error(e.what());
                }
                return Assets::Palette();
            }
        }

        Path TextureLoader::findPalette(const EL::VariableStore& variables, const FileSystem& gameFS, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger) {
            const auto path = buildPalettePath(variables, textureConfig.palette, logger);
            if (!isValidPalettePath(gameFS, path) && !textureConfig.palettefallback.isEmpty()) {
                const auto fallback = buildPalettePath(variables, textureConfig.palettefallback, logger);
                if (isValidPalettePath(gameFS, fallback)) {
                    StringStream msg;
                    msg << "Using fallback palette: " << fallback;
                    logger->info(msg.str());
                    return fallback;
                }
            }
            return path;
        }

        Path TextureLoader::buildPalettePath(const EL::VariableStore& variables, const IO::Path& pathSpec, Logger* logger) {
            try {
                return IO::Path(EL::interpolate(pathSpec.asString(), EL::EvaluationContext(variables)));
            } catch (const Exception& e) {
                logger->warn(e.what());
                return IO::Path();
            }
        }

        bool TextureLoader::isValidPalettePath(const FileSystem& gameFS, const IO::Path& path) {
            return !path.isEmpty() && !path.isAbsolute() && gameFS.fileExists(path);
        }

        TextureLoader::LoaderPtr TextureLoader::createTextureCollectionLoader(const FileSystem& gameFS, const IO::Path::List& fileSearchPaths, const Model::GameConfig::TextureConfig& textureConfig) {
            using Model::GameConfig;
            switch (textureConfig.package.type) {
                case GameConfig::TexturePackageConfig::PT_File:
                    return std::make_unique<FileTextureCollectionLoader>(fileSearchPaths);
                case GameConfig::TexturePackageConfig::PT_Directory:
                    return std::make_unique<DirectoryTextureCollectionLoader>(gameFS);
                case GameConfig::TexturePackageConfig::PT_Unset:
                    throw GameException("Texture package format is not set");
                switchDefault()
            }
        }

        Assets::TextureCollection* TextureLoader::loadTextureCollection(const Path& path) {
            return m_textureCollectionLoader->loadTextureCollection(path, m_textureExtensions, *m_textureReader);
        }

        void TextureLoader::loadTextures(const Path::List& paths, Assets::TextureManager& textureManager) {
            textureManager.setTextureCollections(paths, *this);
        }
    }
}
