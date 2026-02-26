/*
 Copyright (C) 2026 Kristian Duske

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

#include "Macros.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace tb::gl
{
class FontManager;
class ResourceManager;
class ShaderManager;
class VboManager;

struct GlInfo
{
  std::string vendor;
  std::string renderer;
  std::string version;
};

using FindResourceFunc =
  std::function<std::filesystem::path(const std::filesystem::path&)>;

class GlManager
{
private:
  std::unique_ptr<ResourceManager> m_resourceManager;
  std::unique_ptr<ShaderManager> m_shaderManager;
  std::unique_ptr<VboManager> m_vboManager;
  std::unique_ptr<FontManager> m_fontManager;

  static GlInfo m_glInfo;

  bool m_initialized = false;

public:
  explicit GlManager(FindResourceFunc findResourceFunc);
  ~GlManager();

  bool initialize();
  bool initialized() const;

  ResourceManager& resourceManager();
  VboManager& vboManager();
  FontManager& fontManager();
  ShaderManager& shaderManager();

  static const GlInfo& glInfo();

  deleteCopyAndMove(GlManager);
}; // namespace tb::gl

} // namespace tb::gl
