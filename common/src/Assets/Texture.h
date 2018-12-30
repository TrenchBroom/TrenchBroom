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

#ifndef TrenchBroom_Texture
#define TrenchBroom_Texture

#include "ByteBuffer.h"
#include "Color.h"
#include "StringUtils.h"
#include "Renderer/GL.h"

#include <vecmath/forward.h>

#include <utility>
#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureCollection;
        
        typedef Buffer<unsigned char> TextureBuffer;

        enum class TextureType {
            Opaque,
            /**
             * Modifies texture uploading to support mask textures.
             */
            Masked,
            /**
             * Modifies the renderer to render brushes with this texture transparent.
             */
            Transparent
        };

        vm::vec2s sizeAtMipLevel(size_t width, size_t height, size_t level);
        size_t bytesPerPixelForFormat(GLenum format);
        void setMipBufferSize(TextureBuffer::List& buffers, size_t mipLevels, size_t width, size_t height, GLenum format);
        
        class Texture {
        private:
            TextureCollection* m_collection;
            String m_name;
            
            size_t m_width;
            size_t m_height;
            Color m_averageColor;

            size_t m_usageCount;
            bool m_overridden;

            GLenum m_format;
            TextureType m_type;

            mutable GLuint m_textureId;
            mutable TextureBuffer::List m_buffers;
        public:
            Texture(const String& name, size_t width, size_t height, const Color& averageColor, const TextureBuffer& buffer, GLenum format, TextureType type);
            Texture(const String& name, size_t width, size_t height, const Color& averageColor, const TextureBuffer::List& buffers, GLenum format, TextureType type);
            Texture(const String& name, size_t width, size_t height, GLenum format = GL_RGB, TextureType type = TextureType::Opaque);
            ~Texture();

            static TextureType selectTextureType(bool masked, float transparency);

            const String& name() const;
            
            size_t width() const;
            size_t height() const;
            const Color& averageColor() const;
            bool transparent() const;

            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();
            bool overridden() const;
            void setOverridden(const bool overridden);

            bool isPrepared() const;
            void prepare(GLuint textureId, int minFilter, int magFilter);
            void setMode(int minFilter, int magFilter);

            void activate() const;
            void deactivate() const;

        public: // exposed for tests only
            /**
             * Returns the texture data in the format returned by format().
             * Once prepare() is called, this will be an empty vector.
             */
            const TextureBuffer::List& buffersIfUnprepared() const;
            /**
             * Will be one of GL_RGB, GL_BGR, GL_RGBA, GL_BGRA.
             */
            GLenum format() const;
            TextureType type() const;

        private:
            void setCollection(TextureCollection* collection);
            friend class TextureCollection;
        };
    }
}

#endif /* defined(TrenchBroom_Texture) */
