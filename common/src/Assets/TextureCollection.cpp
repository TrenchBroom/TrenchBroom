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

#include "TextureCollection.h"

#include "Ensure.h"

#include <kdl/reflection_impl.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom::Assets
{

kdl_reflect_impl(TextureCollection);

TextureCollection::TextureCollection() = default;

TextureCollection::TextureCollection(std::vector<Texture> textures)
  : m_textures{std::move(textures)}
{
}

TextureCollection::TextureCollection(std::filesystem::path path)
  : m_path{std::move(path)}
{
}

TextureCollection::TextureCollection(
  std::filesystem::path path, std::vector<Texture> textures)
  : m_path{std::move(path)}
  , m_textures{std::move(textures)}
  , m_loaded{true}
{
}

TextureCollection::~TextureCollection()
{
  if (!m_textureIds.empty())
  {
    glAssert(glDeleteTextures(
      static_cast<GLsizei>(m_textureIds.size()),
      static_cast<GLuint*>(&m_textureIds.front())));
    m_textureIds.clear();
  }
}

bool TextureCollection::loaded() const
{
  return m_loaded;
}

const std::filesystem::path& TextureCollection::path() const
{
  return m_path;
}

std::string TextureCollection::name() const
{
  return !m_path.empty() ? m_path.filename().string() : "";
}

size_t TextureCollection::textureCount() const
{
  return m_textures.size();
}

const std::vector<Texture>& TextureCollection::textures() const
{
  return m_textures;
}

std::vector<Texture>& TextureCollection::textures()
{
  return m_textures;
}

const Texture* TextureCollection::textureByIndex(const size_t index) const
{
  return index < m_textures.size() ? &m_textures[index] : nullptr;
}

Texture* TextureCollection::textureByIndex(const size_t index)
{
  return const_cast<Texture*>(
    const_cast<const TextureCollection*>(this)->textureByIndex(index));
}

const Texture* TextureCollection::textureByName(const std::string& name) const
{
  const auto it =
    std::find_if(m_textures.begin(), m_textures.end(), [&](const auto& texture) {
      return texture.name() == name;
    });
  return it != m_textures.end() ? &*it : nullptr;
}

Texture* TextureCollection::textureByName(const std::string& name)
{
  return const_cast<Texture*>(
    const_cast<const TextureCollection*>(this)->textureByName(name));
}

bool TextureCollection::prepared() const
{
  return !m_textureIds.empty();
}

void TextureCollection::prepare(const int minFilter, const int magFilter)
{
  assert(!prepared());

  m_textureIds.resize(textureCount());
  if (textureCount() != 0u)
  {
    glAssert(glGenTextures(
      static_cast<GLsizei>(textureCount()), static_cast<GLuint*>(&m_textureIds.front())));

    for (size_t i = 0; i < textureCount(); ++i)
    {
      auto& texture = m_textures[i];
      texture.prepare(m_textureIds[i], minFilter, magFilter);
    }
  }
}

void TextureCollection::setTextureMode(const int minFilter, const int magFilter)
{
  for (auto& texture : m_textures)
  {
    texture.setMode(minFilter, magFilter);
  }
}

} // namespace TrenchBroom::Assets
