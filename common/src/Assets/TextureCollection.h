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

#pragma once

#include "Assets/Texture.h"
#include "Renderer/GL.h"

#include "kdl/reflection_decl.h"

#include <filesystem>
#include <string>
#include <vector>

namespace TrenchBroom::Assets
{

class TextureCollection
{
private:
  using TextureIdList = std::vector<GLuint>;

  std::filesystem::path m_path;
  std::vector<Texture> m_textures;

  bool m_loaded{false};
  TextureIdList m_textureIds;

  friend class Texture;

  kdl_reflect_decl(TextureCollection, m_loaded, m_path, m_textures);

public:
  TextureCollection();
  explicit TextureCollection(std::vector<Texture> textures);
  explicit TextureCollection(std::filesystem::path path);
  TextureCollection(std::filesystem::path path, std::vector<Texture> textures);

  TextureCollection(const TextureCollection&) = delete;
  TextureCollection& operator=(const TextureCollection&) = delete;

  TextureCollection(TextureCollection&& other) = default;
  TextureCollection& operator=(TextureCollection&& other) = default;

  ~TextureCollection();

  bool loaded() const;
  const std::filesystem::path& path() const;
  size_t textureCount() const;

  const std::vector<Texture>& textures() const;
  std::vector<Texture>& textures();

  const Texture* textureByIndex(size_t index) const;
  Texture* textureByIndex(size_t index);

  const Texture* textureByName(const std::string& name) const;
  Texture* textureByName(const std::string& name);

  bool prepared() const;
  void prepare(int minFilter, int magFilter);
  void setTextureMode(int minFilter, int magFilter);
};

} // namespace TrenchBroom::Assets
