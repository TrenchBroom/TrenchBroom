/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__AutoTexture__
#define __TrenchBroom__AutoTexture__

#include "Renderer/GL.h"
#include "ByteBuffer.h"
#include "SharedPointer.h"
#include "Assets/Texture.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class AutoTexture : public Texture {
        private:
            mutable GLuint m_textureId;
            size_t m_width;
            size_t m_height;
            Color m_averageColor;
            mutable Buffer<unsigned char>::List m_buffers;
        public:
            AutoTexture(const size_t width, const size_t height, const Color& averageColor, const Buffer<unsigned char>& buffer);
            AutoTexture(const size_t width, const size_t height, const Color& averageColor, const Buffer<unsigned char>::List& buffers);
            ~AutoTexture();
            
            size_t width() const;
            size_t height() const;
            const Color& averageColor() const;
            
            void activate() const;
            void deactivate() const;
        private:
            void deleteBuffers() const;
        };
    }
}

#endif /* defined(__TrenchBroom__AutoTexture__) */
