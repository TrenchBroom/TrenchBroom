/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Texture_h
#define TrenchBroom_Texture_h

#include <string>
#include <vector>
#include <map>

#include "IO/Wad.h"
#include "Model/Assets/Alias.h"
#include "Model/Assets/Bsp.h"
#include "Model/Assets/Palette.h"
#include "Utilities/Event.h"
#include "GL/GLee.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
            static const std::string TextureManagerChanged = "TextureManagerChanged";
            static bool compareByName(const Texture* texture1, const Texture* texture2);
            static bool compareByUsageCount(const Texture* texture1, const Texture* texture2);

            typedef enum {
                TB_TS_NAME,
                TB_TS_USAGE
            } ETextureSortCriterion;

            class Texture {
            private:
                GLuint m_textureId;
                unsigned char* m_textureBuffer;

                void init(const std::string& name, unsigned int width, unsigned int height);
                void init(const std::string& name, const unsigned char* indexImage, unsigned int width, unsigned int height, const Palette* palette);
            public:
                std::string name;
                int uniqueId;
                unsigned int usageCount;
                unsigned int width;
                unsigned int height;
                Vec4f averageColor;
                bool dummy;

                Texture(const std::string& name, const unsigned char* rgbImage, unsigned int width, unsigned int height);
                Texture(const std::string& name, const unsigned char* indexedImage, unsigned int width, unsigned int height, const Palette& palette);
                Texture(const IO::Mip& mip, const Palette& palette);
                Texture(const std::string& name, const AliasSkin& skin, unsigned int skinIndex, const Palette& palette);
                Texture(const std::string& name, const BspTexture& texture, const Palette& palette);
                Texture(const std::string& name);
                ~Texture();

                void activate();
                void deactivate();
            };

            class TextureCollection {
            private:
                std::vector<Texture*> m_textures;
                std::string m_name;
            public:
                TextureCollection(const std::string& name, IO::Wad& wad, const Palette& palette);
                ~TextureCollection();
                const std::vector<Texture*>& textures() const;
                std::vector<Texture*> textures(ETextureSortCriterion criterion) const;
                const std::string& name() const;
            };

            class TextureManager {
            private:
                typedef std::map<std::string, Texture*> TextureMap;
                
                std::vector<TextureCollection*> m_collections;
                TextureMap m_texturesCaseSensitive;
                TextureMap m_texturesCaseInsensitive;
                void reloadTextures();
            public:
                typedef Event<TextureManager&> TextureManagerEvent;
                TextureManagerEvent textureManagerDidChange;

                ~TextureManager();

                void addCollection(TextureCollection* collection, unsigned int index);
                void removeCollection(unsigned int index);
                void clear();

                const std::vector<TextureCollection*>& collections();
                const std::vector<Texture*> textures(ETextureSortCriterion criterion);
                Texture* texture(const std::string& name);

                void activateTexture(const std::string& name);
                void deactivateTexture();
            };
        }
    }
}
#endif
