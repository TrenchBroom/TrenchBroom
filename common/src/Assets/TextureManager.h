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

#pragma once

#include "Assets/TextureCollection.h"

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
        class TextureLoader;
    }

    namespace Assets {
        class Texture;
        class TextureCollection;

        class TextureManager {
        private:
            using TextureMap = std::map<std::string, Texture*>;

            Logger& m_logger;

            std::vector<TextureCollection> m_collections;

            std::vector<size_t> m_toPrepare;
            std::vector<TextureCollection> m_toRemove;

            TextureMap m_texturesByName;
            std::vector<const Texture*> m_textures;

            int m_minFilter;
            int m_magFilter;
            bool m_resetTextureMode;
        public:
            TextureManager(int magFilter, int minFilter, Logger& logger);
            ~TextureManager();

            void setTextureCollections(const std::vector<IO::Path>& paths, IO::TextureLoader& loader);
            void setTextureCollections(std::vector<TextureCollection> collections);
        private:
            void addTextureCollection(Assets::TextureCollection collection);
        public:
            void clear();

            void setTextureMode(int minFilter, int magFilter);
            void commitChanges();

            const Texture* texture(const std::string& name) const;
            Texture* texture(const std::string& name);
            
            const std::vector<const Texture*>& textures() const;
            const std::vector<TextureCollection>& collections() const;
        private:
            void resetTextureMode();
            void prepare();

            void updateTextures();
        };
    }
}


