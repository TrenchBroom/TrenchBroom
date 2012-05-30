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

using namespace std;

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
            static const string TextureManagerChanged = "TextureManagerChanged";
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

                void init(const string& name, unsigned int width, unsigned int height);
                void init(const string& name, const unsigned char* indexImage, unsigned int width, unsigned int height, const Palette* palette);
            public:
                string name;
                int uniqueId;
                unsigned int usageCount;
                unsigned int width;
                unsigned int height;
                Vec4f averageColor;
                bool dummy;

                Texture(const string& name, const unsigned char* rgbImage, unsigned int width, unsigned int height);
                Texture(const string& name, const unsigned char* indexedImage, unsigned int width, unsigned int height, const Palette& palette);
                Texture(const IO::Mip& mip, const Palette& palette);
                Texture(const string& name, const AliasSkin& skin, unsigned int skinIndex, const Palette& palette);
                Texture(const string& name, const BspTexture& texture, const Palette& palette);
                Texture(const string& name);
                ~Texture();

                void activate();
                void deactivate();
            };

            class TextureCollection {
            private:
                vector<Texture*> m_textures;
                string m_name;
            public:
                TextureCollection(const string& name, IO::Wad& wad, const Palette& palette);
                ~TextureCollection();
                const vector<Texture*>& textures() const;
                vector<Texture*> textures(ETextureSortCriterion criterion) const;
                const std::string& name() const;
            };

            class TextureManager {
            private:
                vector<TextureCollection*> m_collections;
                map<string, Texture*> m_textures;
                void reloadTextures();
            public:
                typedef Event<TextureManager&> TextureManagerEvent;
                TextureManagerEvent textureManagerDidChange;

                ~TextureManager();

                void addCollection(TextureCollection* collection, unsigned int index);
                void removeCollection(unsigned int index);
                void clear();

                const vector<TextureCollection*>& collections();
                const vector<Texture*> textures(ETextureSortCriterion criterion);
                Texture* texture(const string& name);

                void activateTexture(const string& name);
                void deactivateTexture();
            };
        }
    }
}
#endif
