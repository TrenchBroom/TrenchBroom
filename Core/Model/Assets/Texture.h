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
#include <OpenGL/gl.h>
#include "VecMath.h"
#include "Wad.h"
#include "Palette.h"
#include "Alias.h"
#include "Bsp.h"
#include "Observer.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            static const string TextureManagerChanged = "TextureManagerChanged";
            
            typedef enum {
                TS_NAME,
                TS_USAGE
            } ETextureSortCriterion;
            
            class Texture {
            private:
                GLuint m_textureId;
                unsigned char* m_textureBuffer;
                
                void init(const string& name, const unsigned char* image, int width, int height, const Palette* palette);
            public:
                string name;
                int uniqueId;
                bool dummy;
                int usageCount;
                int width;
                int height;
                Vec4f averageColor;
                
                Texture(const string& name, const unsigned char* image, int width, int height, const Palette& palette);
                Texture(const IO::Mip& mip, const Palette& palette);
                Texture(const string& name, const AliasSkin& skin, int skinIndex, const Palette& palette);
                Texture(const string& name, const BspTexture& texture, const Palette& palette);
                Texture(const string& name);
                ~Texture();
                
                void activate();
                void deactivate();
            };
            
            class TextureCollection {
            public:
                vector<Texture*> textures;
                string name;
                TextureCollection(const string& name, IO::Wad& wad, const Palette& palette);
                ~TextureCollection();
            };
            
            class TextureManager : public Observable {
            private:
                vector<TextureCollection*> m_collections;
                map<string, Texture*> m_textures;
                map<string, Texture*> m_dummies;
                void reloadTextures();
            public:
                ~TextureManager();
                
                void addCollection(TextureCollection* collection, int index);
                void removeCollection(int index);
                void clear();
                
                const vector<TextureCollection*> collections();
                const vector<Texture*> textures(ETextureSortCriterion criterion);
                Texture* texture(const string& name);
                
                void activateTexture(const string& name);
                void deactivateTexture();
            };
        }
    }
}
#endif
