/*
 Copyright (C) 2010 Kristian Duske

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

#include <memory>
#include <string>

namespace tb::render
{
class FontManager;
class ShaderManager;
class VboManager;
} // namespace tb::render

namespace tb::ui
{

class GLContextManager
{
public:
  static std::string GLVendor;
  static std::string GLRenderer;
  static std::string GLVersion;

private:
  std::string m_glVendor;
  std::string m_glRenderer;
  std::string m_glVersion;

  std::unique_ptr<render::ShaderManager> m_shaderManager;
  std::unique_ptr<render::VboManager> m_vboManager;
  std::unique_ptr<render::FontManager> m_fontManager;

  bool m_initialized = false;

public:
  GLContextManager();
  ~GLContextManager();

  bool initialized() const;
  bool initialize();

  render::VboManager& vboManager();
  render::FontManager& fontManager();
  render::ShaderManager& shaderManager();

  deleteCopyAndMove(GLContextManager);
};

} // namespace tb::ui
