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

#pragma once

#include "Assets/TextureBuffer.h"
#include "Color.h"
#include "Renderer/GL.h"

#include "kdl/reflection_decl.h"

#include <variant>
#include <vector>

namespace TrenchBroom::Assets
{

enum class TextureMask
{
  /**
   * Modifies texture uploading to support mask textures.
   */
  On,
  Off,
};

std::ostream& operator<<(std::ostream& lhs, const TextureMask& rhs);

struct NoEmbeddedDefaults
{
  kdl_reflect_decl_empty(NoEmbeddedDefaults);
};

struct Q2EmbeddedDefaults
{
  int flags;
  int contents;
  int value;

  kdl_reflect_decl(Q2EmbeddedDefaults, flags, contents, value);
};

using EmbeddedDefaults = std::variant<NoEmbeddedDefaults, Q2EmbeddedDefaults>;

std::ostream& operator<<(std::ostream& lhs, const EmbeddedDefaults& rhs);

struct TextureLoadedState
{
  std::vector<TextureBuffer> buffers;

  kdl_reflect_decl(TextureLoadedState, buffers);
};

struct TextureReadyState
{
  GLuint textureId;

  kdl_reflect_decl(TextureReadyState, textureId);
};

struct TextureDroppedState
{
  kdl_reflect_decl_empty(TextureDroppedState);
};

using TextureState =
  std::variant<TextureLoadedState, TextureReadyState, TextureDroppedState>;

class Texture
{
private:
  size_t m_width;
  size_t m_height;
  Color m_averageColor;

  GLenum m_format;
  TextureMask m_mask;

  EmbeddedDefaults m_embeddedDefaults;

  mutable TextureState m_state;

  kdl_reflect_decl(
    Texture,
    m_width,
    m_height,
    m_averageColor,
    m_format,
    m_mask,
    m_embeddedDefaults,
    m_state);

public:
  Texture(
    size_t width,
    size_t height,
    Color averageColor,
    GLenum format,
    TextureMask mask,
    EmbeddedDefaults embeddedDefaults,
    std::vector<TextureBuffer> buffers);

  Texture(
    size_t width,
    size_t height,
    Color averageColor,
    GLenum format,
    TextureMask mask,
    EmbeddedDefaults embeddedDefaults,
    TextureBuffer buffer);

  Texture(size_t width, size_t height);

  Texture(const Texture& other) = delete;
  Texture(Texture&& other) = default;

  Texture& operator=(const Texture& other) = delete;
  Texture& operator=(Texture&& other) = default;

  ~Texture() = default;

  size_t width() const;
  size_t height() const;
  vm::vec2f sizef() const;
  const Color& averageColor() const;

  GLenum format() const;

  TextureMask mask() const;
  void setMask(TextureMask mask);

  const EmbeddedDefaults& embeddedDefaults() const;

  bool isReady() const;
  void setFilterMode(int minFilter, int magFilter);

  bool activate() const;
  bool deactivate() const;

  void upload(bool glContextAvailable);
  void drop(bool glContextAvailable);

  const std::vector<TextureBuffer>& buffersIfLoaded() const;
};


} // namespace TrenchBroom::Assets
