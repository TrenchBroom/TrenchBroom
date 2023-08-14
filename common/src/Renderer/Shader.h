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

#include "Macros.h"
#include "Renderer/GL.h"

#include <kdl/result_forward.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
struct Error;
}

namespace TrenchBroom::Renderer
{

class Shader
{
private:
  std::string m_name;
  GLenum m_type;
  GLuint m_shaderId;

public:
  Shader(std::string name, GLenum type, GLuint shaderId);

  deleteCopy(Shader);

  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  ~Shader();

  void attach(GLuint programId) const;
};

kdl::result<Shader, Error> loadShader(const std::filesystem::path& path, GLenum type);

} // namespace TrenchBroom::Renderer
