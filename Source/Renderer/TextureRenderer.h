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

#ifndef __TrenchBroom__TextureRenderer__
#define __TrenchBroom__TextureRenderer__

#include <GL/glew.h>
#include "Utility/Color.h"

namespace TrenchBroom {
    namespace IO {
        class Mip;
    }
    
    namespace Model {
        class AliasSkin;
        class BspTexture;
    }
    
    namespace Renderer {
        class Palette;
        
        class TextureRenderer {
        protected:
            GLuint m_textureId;
            unsigned int m_width;
            unsigned int m_height;
            unsigned char* m_textureBuffer;
            Color m_averageColor;
            
            void init(unsigned int width, unsigned int height);
            void init(unsigned char* rgbImage, unsigned int width, unsigned int height);

            // prevent copying
            TextureRenderer(const TextureRenderer& other);
            void operator= (const TextureRenderer& other);
        public:
            TextureRenderer(unsigned char* rgbImage, const Color& averageColor, unsigned int width, unsigned int height);
            TextureRenderer(const Model::AliasSkin& skin, unsigned int skinIndex, const Palette& palette);
            TextureRenderer(const Model::BspTexture& texture, const Palette& palette);
            TextureRenderer();
            ~TextureRenderer();

            inline const Color& averageColor() const {
                return m_averageColor;
            }
            
            void activate();
            void deactivate();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureRenderer__) */
