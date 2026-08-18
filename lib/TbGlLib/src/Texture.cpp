/*
 Copyright (C) 2024 Kristian Duske

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

#include "gl/Texture.h"

#include "gl/GlInterface.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/reflection_impl.h"
#include "kd/vector_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

namespace tb::gl
{

namespace
{

auto makeTextureLoadedState(
  const size_t width,
  const size_t height,
  const GLenum format,
  std::vector<TextureBuffer> buffers)
{
  const auto compressed = isCompressedFormat(format);
  [[maybe_unused]] const auto bytesPerPixel =
    compressed ? 0U : bytesPerPixelForFormat(format);
  [[maybe_unused]] const auto blockSize = compressed ? blockSizeForFormat(format) : 0U;

  for (size_t level = 0; level < buffers.size(); ++level)
  {
    [[maybe_unused]] const auto mipSize = sizeAtMipLevel(width, height, level);
    [[maybe_unused]] const auto numBytes =
      compressed ? (blockSize * std::max(size_t(1), mipSize.x() / 4)
                    * std::max(size_t(1), mipSize.y() / 4))
                 : (bytesPerPixel * mipSize.x() * mipSize.y());
    contract_assert(buffers[level].size() >= numBytes);
  }

  return TextureLoadedState{std::move(buffers)};
}

auto uploadTexture(
  Gl& gl,
  const GLenum format,
  const bool disableMipMaps,
  const std::vector<TextureBuffer>& buffers,
  const size_t width,
  const size_t height)
{
  const auto compressed = isCompressedFormat(format);
  auto useMipmap = false;

  auto textureId = GLuint(0);
  gl.genTextures(1, &textureId);

  gl.pixelStorei(GL_UNPACK_SWAP_BYTES, false);
  gl.pixelStorei(GL_UNPACK_LSB_FIRST, false);
  gl.pixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  gl.pixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  gl.pixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  gl.pixelStorei(GL_UNPACK_ALIGNMENT, 1);

  gl.bindTexture(GL_TEXTURE_2D, textureId);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if (disableMipMaps)
  {
    // masked textures don't work well with automatic mipmaps, so we force
    // GL_NEAREST filtering
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  else
  {
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(buffers.size() - 1));
    useMipmap = buffers.size() > 1;
  }

  // Upload only the first mipmap for masked textures.
  const auto mipmapsToUpload = disableMipMaps ? 1u : buffers.size();

  for (size_t j = 0; j < mipmapsToUpload; ++j)
  {
    const auto mipSize = sizeAtMipLevel(width, height, j);

    const auto* data = reinterpret_cast<const GLvoid*>(buffers[j].data());
    if (compressed)
    {
      const auto dataSize = GLsizei(buffers[j].size());

      gl.compressedTexImage2D(
        GL_TEXTURE_2D,
        GLint(j),
        format,
        GLsizei(mipSize.x()),
        GLsizei(mipSize.y()),
        0,
        dataSize,
        data);
    }
    else
    {
      gl.texImage2D(
        GL_TEXTURE_2D,
        GLint(j),
        GL_RGBA,
        GLsizei(mipSize.x()),
        GLsizei(mipSize.y()),
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);
    }
  }

  return std::tuple{textureId, useMipmap};
}

void dropTexture(Gl& gl, GLuint textureId)
{
  gl.deleteTextures(1, &textureId);
}

auto textureFilterMode(const int filter, const bool useMipmap)
{
  switch (filter)
  {
  case GL_LINEAR_MIPMAP_NEAREST:
  case GL_LINEAR_MIPMAP_LINEAR:
    return useMipmap ? filter : GL_LINEAR;
  case GL_NEAREST_MIPMAP_NEAREST:
  case GL_NEAREST_MIPMAP_LINEAR:
    return useMipmap ? filter : GL_NEAREST;
  default:
    return filter;
  }
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const EmbeddedDefaults& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

kdl_reflect_impl(NoEmbeddedDefaults);
kdl_reflect_impl(Q2EmbeddedDefaults);

kdl_reflect_impl(TextureLoadedState);
kdl_reflect_impl(TextureReadyState);
kdl_reflect_impl(TextureDroppedState);

kdl_reflect_impl(Texture);

Texture::Texture(
  const size_t width,
  const size_t height,
  const Color averageColor,
  const GLenum format,
  EmbeddedDefaults embeddedDefaults,
  std::vector<TextureBuffer> buffers)
  : m_width{width}
  , m_height{height}
  , m_averageColor{averageColor}
  , m_format{format}
  , m_embeddedDefaults{std::move(embeddedDefaults)}
  , m_state{makeTextureLoadedState(m_width, m_height, m_format, std::move(buffers))}
{
  contract_pre(m_width > 0);
  contract_pre(m_height > 0);
}

Texture::Texture(
  const size_t width,
  const size_t height,
  const Color averageColor,
  const GLenum format,
  EmbeddedDefaults embeddedDefaults,
  TextureBuffer buffer)
  : Texture{
      width,
      height,
      averageColor,
      format,
      std::move(embeddedDefaults),
      kdl::vec_from(std::move(buffer))}
{
}

Texture::Texture(size_t width, size_t height)
  : Texture{
      width, height, RgbaF{}, GL_RGBA, NoEmbeddedDefaults{}, std::vector<TextureBuffer>{}}
{
}

size_t Texture::width() const
{
  return m_width;
}

size_t Texture::height() const
{
  return m_height;
}

vm::vec2f Texture::sizef() const
{
  return vm::vec2f(float(m_width), float(m_height));
}

const Color& Texture::averageColor() const
{
  return m_averageColor;
}

GLenum Texture::format() const
{
  return m_format;
}

img::ImageAlphaDomain Texture::alphaDomain() const
{
  return m_alphaDomain;
}

void Texture::setAlphaDomain(const img::ImageAlphaDomain alphaDomain)
{
  m_alphaDomain = alphaDomain;
}

const EmbeddedDefaults& Texture::embeddedDefaults() const
{
  return m_embeddedDefaults;
}

bool Texture::isReady() const
{
  return std::holds_alternative<TextureReadyState>(m_state);
}

bool Texture::activate(Gl& gl, const int minFilter, const int magFilter) const
{
  return std::visit(
    kdl::overload(
      [](const TextureLoadedState&) { return false; },
      [&](const TextureReadyState& readyState) {
        gl.bindTexture(GL_TEXTURE_2D, readyState.textureId);
        setFilterMode(gl, minFilter, magFilter, readyState.useMipmap);
        return true;
      },
      [](const TextureDroppedState&) { return false; }),
    m_state);
}

bool Texture::deactivate(Gl& gl) const
{
  if (isReady())
  {
    gl.bindTexture(GL_TEXTURE_2D, 0);
    return true;
  }

  return false;
}

void Texture::upload(Gl& gl)
{
  m_state = std::visit(
    kdl::overload(
      [&](const TextureLoadedState& textureLoadedState) -> TextureState {
        // Skip mipmaps and force GL_NEAREST filtering for a hard alpha cutout, to
        // avoid it blurring into a semi-transparent fringe.
        const auto disableMipMaps = m_alphaDomain == img::ImageAlphaDomain::Binary;
        const auto [textureId, useMipmap] = uploadTexture(
          gl, m_format, disableMipMaps, textureLoadedState.buffers, m_width, m_height);
        return TextureReadyState{textureId, useMipmap};
      },
      [](TextureReadyState textureReadyState) -> TextureState {
        return textureReadyState;
      },
      [](TextureDroppedState textureDroppedState) -> TextureState {
        return textureDroppedState;
      }),
    std::move(m_state));
}

void Texture::drop(Gl& gl)
{
  m_state = std::visit(
    kdl::overload(
      [&](const TextureLoadedState&) { return TextureDroppedState{}; },
      [&](const TextureReadyState& textureReadyState) {
        dropTexture(gl, textureReadyState.textureId);
        return TextureDroppedState{};
      },
      [](TextureDroppedState textureDroppedState) { return textureDroppedState; }),
    std::move(m_state));
}

const std::vector<TextureBuffer>& Texture::buffersIfLoaded() const
{
  static const auto empty = std::vector<TextureBuffer>{};
  return std::visit(
    kdl::overload(
      [](const TextureLoadedState& state) -> const std::vector<TextureBuffer>& {
        return state.buffers;
      },
      [](const TextureReadyState&) -> const std::vector<TextureBuffer>& { return empty; },
      [](const TextureDroppedState&) -> const std::vector<TextureBuffer>& {
        return empty;
      }),
    m_state);
}

void Texture::setFilterMode(
  Gl& gl, const int minFilter, const int magFilter, const bool useMipmap) const
{
  if (m_alphaDomain == img::ImageAlphaDomain::Binary)
  {
    // Force GL_NEAREST filtering for a hard alpha cutout, to avoid it blurring into a
    // semi-transparent fringe.
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  else
  {
    gl.texParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterMode(minFilter, useMipmap));
    gl.texParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterMode(magFilter, useMipmap));
  }
}

} // namespace tb::gl
