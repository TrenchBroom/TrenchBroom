/*
 Copyright (C) 2020 MaxED

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

#include "EntitySpriteManager.h"

#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/SystemPaths.h"

namespace TrenchBroom::Assets {
    EntitySpriteManager::EntitySpriteManager(const int magFilter, const int minFilter, Logger& logger) :
        m_logger(logger),
        m_minFilter(minFilter),
        m_magFilter(magFilter),
        m_resetTextureMode(false) {}

    EntitySpriteManager::~EntitySpriteManager() = default;

    const Texture* EntitySpriteManager::sprite(const std::string& name) const {
        return m_spriteCollection.textureByName(kdl::str_to_lower(name));
    }

    void EntitySpriteManager::load() {
    	// Create and check sprites folder...
        auto spritesPath = IO::SystemPaths::appDirectory() + IO::Path("defaults/assets/sprites");
    	
        if (!IO::Disk::directoryExists(spritesPath)) {
            m_logger.error() << "Could not load entity sprites from '" << spritesPath << "'";
        	return;
        }

    	// Load images...
        m_logger.info() << "Loading entity sprites from '" << spritesPath << "'...";

        const IO::DiskFileSystem diskFS(spritesPath);
        const auto imagePaths = diskFS.findItemsRecursively(IO::Path(), IO::FileExtensionMatcher(std::vector<std::string> { "tga", "png" }));

        auto textures = std::vector<Assets::Texture>();
        textures.reserve(imagePaths.size());

        const auto spriteReader = IO::FreeImageTextureReader(IO::TextureReader::SpriteNameStrategy(), diskFS, m_logger);

        for (const auto& imagePath : imagePaths) {
            try {
                const auto file = diskFS.openFile(imagePath);
                textures.push_back(spriteReader.readTexture(file));
            } catch (const std::exception& e) {
                m_logger.warn() << e.what();
            }
        }

    	// Create sprites collection...
        m_spriteCollection = TextureCollection(spritesPath, std::move(textures));
        m_logger.info() << "Loaded " << m_spriteCollection.textureCount() << " entity sprites";
    }

    void EntitySpriteManager::setTextureMode(const int minFilter, const int magFilter) {
        m_minFilter = minFilter;
        m_magFilter = magFilter;
        m_resetTextureMode = true;
    }

    void EntitySpriteManager::commitChanges() {
        // Reset texture mode
        if (m_resetTextureMode) {
            m_spriteCollection.setTextureMode(m_minFilter, m_magFilter);
            m_resetTextureMode = false;
        }

    	// Prepare collection
    	if (!m_spriteCollection.prepared()) {
            m_spriteCollection.prepare(m_minFilter, m_magFilter, TextureWrap::Clamp);
    	}
    }
}
