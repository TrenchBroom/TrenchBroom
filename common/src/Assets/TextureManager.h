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

#ifndef TrenchBroom_TextureManager
#define TrenchBroom_TextureManager

#include "Notifier.h"

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
            using TextureCollectionMap = std::map<IO::Path, TextureCollection*>;
            using TextureCollectionMapEntry = std::pair<IO::Path, TextureCollection*>;
            using TextureMap = std::map<std::string, Texture*>;

            Logger& m_logger;

            std::vector<TextureCollection*> m_collections;

            std::vector<TextureCollection*> m_toPrepare;
            std::vector<TextureCollection*> m_toRemove;

            TextureMap m_texturesByName;
            std::vector<const Texture*> m_textures;

            int m_minFilter;
            int m_magFilter;
            bool m_resetTextureMode;
        public:
            Notifier<> usageCountDidChange;
        public:
            TextureManager(int magFilter, int minFilter, Logger& logger);
            ~TextureManager();

            void setTextureCollections(const std::vector<IO::Path>& paths, IO::TextureLoader& loader);
            void setTextureCollections(const std::vector<TextureCollection*>& collections);
        private:
            TextureCollectionMap collectionMap() const;
            void addTextureCollection(Assets::TextureCollection* collection);
        public:
            void clear();

            void setTextureMode(int minFilter, int magFilter);
            void commitChanges();

            const Texture* texture(const std::string& name) const;
            Texture* texture(const std::string& name);
            
            const std::vector<const Texture*>& textures() const;
            const std::vector<TextureCollection*>& collections() const;
            const std::vector<std::string> collectionNames() const;
        private:
            void resetTextureMode();
            void prepare();

            void updateTextures();
        };
    }
}

#endif /* defined(TrenchBroom_TextureManager) */
