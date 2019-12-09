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

#include "Texture.h"
#include "Assets/TextureBuffer.h"
#include "Assets/TextureCollection.h"
#include "Renderer/GL.h"

#include <algorithm> // for std::max
#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        Texture::Texture(const std::string& name, const size_t width, const size_t height, const Color& averageColor, Buffer&& buffer, const GLenum format, const TextureType type) :
        m_collection(nullptr),
        m_name(name),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor),
        m_usageCount(0),
        m_overridden(false),
        m_format(format),
        m_type(type),
        m_culling(TextureCulling::CullDefault),
        m_blendFunc{false, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA},
        m_textureId(0) {
            assert(m_width > 0);
            assert(m_height > 0);
            assert(buffer.size() >= m_width * m_height * bytesPerPixelForFormat(format));
            m_buffers.push_back(std::move(buffer));
        }

        Texture::Texture(const std::string& name, const size_t width, const size_t height, const Color& averageColor, BufferList&& buffers, const GLenum format, const TextureType type) :
        m_collection(nullptr),
        m_name(name),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor),
        m_usageCount(0),
        m_overridden(false),
        m_format(format),
        m_type(type),
        m_culling(TextureCulling::CullDefault),
        m_blendFunc{false, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA},
        m_textureId(0),
        m_buffers(std::move(buffers)) {
            assert(m_width > 0);
            assert(m_height > 0);

            [[maybe_unused]] const auto bytesPerPixel = bytesPerPixelForFormat(format);

            for (size_t level = 0; level < m_buffers.size(); ++level) {
                [[maybe_unused]] const auto mipSize = sizeAtMipLevel(m_width, m_height, level);
                [[maybe_unused]] const auto numBytes = bytesPerPixel * mipSize.x() * mipSize.y();
                assert(m_buffers[level].size() >= numBytes);
            }
        }

        Texture::Texture(const std::string& name, const size_t width, const size_t height, const GLenum format, const TextureType type) :
        m_collection(nullptr),
        m_name(name),
        m_width(width),
        m_height(height),
        m_averageColor(Color(0.0f, 0.0f, 0.0f, 1.0f)),
        m_usageCount(0),
        m_overridden(false),
        m_format(format),
        m_type(type),
        m_culling(TextureCulling::CullDefault),
        m_blendFunc{false, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA},
        m_textureId(0) {}

        Texture::~Texture() {
            if (m_collection == nullptr && m_textureId != 0) {
                glAssert(glDeleteTextures(1, &m_textureId));
            }
            m_textureId = 0;
        }

        TextureType Texture::selectTextureType(const bool masked) {
            if (masked) {
                return TextureType::Masked;
            } else {
                return TextureType::Opaque;
            }
        }

        TextureCollection* Texture::collection() const {
            return m_collection;
        }

        const std::string& Texture::name() const {
            return m_name;
        }

        size_t Texture::width() const {
            return m_width;
        }

        size_t Texture::height() const {
            return m_height;
        }

        const Color& Texture::averageColor() const {
            return m_averageColor;
        }

        const std::set<std::string>& Texture::surfaceParms() const {
            return m_surfaceParms;
        }

        void Texture::setSurfaceParms(const std::set<std::string>& surfaceParms) {
            m_surfaceParms = surfaceParms;
        }

        TextureCulling Texture::culling() const {
            return m_culling;
        }

        void Texture::setCulling(const TextureCulling culling) {
            m_culling = culling;
        }

        const TextureBlendFunc& Texture::blendFunc() const {
            return m_blendFunc;
        }

        void Texture::setBlendFunc(GLenum srcFactor, GLenum destFactor) {
            m_blendFunc.enable = true;
            m_blendFunc.srcFactor = srcFactor;
            m_blendFunc.destFactor = destFactor;
        }

        size_t Texture::usageCount() const {
            return m_usageCount;
        }

        void Texture::incUsageCount() {
            ++m_usageCount;
            if (m_collection != nullptr) {
                m_collection->incUsageCount();
            }
        }

        void Texture::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
            if (m_collection != nullptr) {
                m_collection->decUsageCount();
            }
        }

        bool Texture::overridden() const {
            return m_overridden;
        }

        void Texture::setOverridden(const bool overridden) {
            m_overridden = overridden;
        }

        bool Texture::isPrepared() const {
            return m_textureId != 0;
        }

        void Texture::prepare(const GLuint textureId, const int minFilter, const int magFilter) {
            assert(textureId > 0);
            assert(m_textureId == 0);

            if (!m_buffers.empty()) {
                glAssert(glPixelStorei(GL_UNPACK_SWAP_BYTES, false));
                glAssert(glPixelStorei(GL_UNPACK_LSB_FIRST, false));
                glAssert(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
                glAssert(glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0));
                glAssert(glPixelStorei(GL_UNPACK_SKIP_ROWS, 0));
                glAssert(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

                glAssert(glBindTexture(GL_TEXTURE_2D, textureId));
                glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
                glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
                glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

                if (m_type == TextureType::Masked) {
                    // masked textures don't work well with automatic mipmaps, so we force GL_NEAREST filtering and don't generate any
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE));
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                } else if (m_buffers.size() == 1) {
                    // generate mipmaps if we don't have any
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));
                } else {
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(m_buffers.size() - 1)));
                }

                // Upload only the first mipmap for masked textures.
                const auto mipmapsToUpload = (m_type == TextureType::Masked) ? 1u : m_buffers.size();

                for (size_t j = 0; j < mipmapsToUpload; ++j) {
                    const auto mipSize = sizeAtMipLevel(m_width, m_height, j);

                    const GLvoid* data = reinterpret_cast<const GLvoid*>(m_buffers[j].data());
                    glAssert(glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(j), GL_RGBA,
                                          static_cast<GLsizei>(mipSize.x()),
                                          static_cast<GLsizei>(mipSize.y()),
                                          0, m_format, GL_UNSIGNED_BYTE, data));
                }

                m_buffers.clear();
                m_textureId = textureId;
            }
        }

        void Texture::setMode(const int minFilter, const int magFilter) {
            if (isPrepared()) {
                activate();
                if (m_type == TextureType::Masked) {
                    // Force GL_NEAREST filtering for masked textures.
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                } else {
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
                    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
                }
                deactivate();
            }
        }

        void Texture::activate() const {
            if (isPrepared()) {
                glAssert(glBindTexture(GL_TEXTURE_2D, m_textureId));

                switch (m_culling) {
                    case Assets::TextureCulling::CullNone:
                        glAssert(glDisable(GL_CULL_FACE));
                        break;
                    case Assets::TextureCulling::CullFront:
                        glAssert(glCullFace(GL_FRONT));
                        break;
                    case Assets::TextureCulling::CullBoth:
                        glAssert(glCullFace(GL_FRONT_AND_BACK));
                        break;
                    case Assets::TextureCulling::CullDefault:
                    case Assets::TextureCulling::CullBack:
                        break;
                }

                if (m_blendFunc.enable) {
                    glAssert(glPushAttrib(GL_COLOR_BUFFER_BIT));
                    glAssert(glBlendFunc(m_blendFunc.srcFactor, m_blendFunc.destFactor));
                }
            }
        }

        void Texture::deactivate() const {
            if (isPrepared()) {
                if (m_blendFunc.enable) {
                    glAssert(glPopAttrib());
                }

                switch (m_culling) {
                    case Assets::TextureCulling::CullNone:
                        glAssert(glEnable(GL_CULL_FACE));
                        break;
                    case Assets::TextureCulling::CullFront:
                        glAssert(glCullFace(GL_BACK));
                        break;
                    case Assets::TextureCulling::CullBoth:
                        glAssert(glCullFace(GL_BACK));
                        break;
                    case Assets::TextureCulling::CullDefault:
                    case Assets::TextureCulling::CullBack:
                        break;
                }

                glAssert(glBindTexture(GL_TEXTURE_2D, 0));
            }
        }

        const Texture::BufferList& Texture::buffersIfUnprepared() const {
            return m_buffers;
        }

        GLenum Texture::format() const {
            return m_format;
        }

        TextureType Texture::type() const {
            return m_type;
        }

        void Texture::setCollection(TextureCollection* collection) {
            m_collection = collection;
        }
    }
}
