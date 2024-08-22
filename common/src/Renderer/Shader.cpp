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

#include "Shader.h"

#include "Error.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom::Renderer
{

Shader::Shader(std::string name, const GLenum type, const GLuint shaderId)
  : m_name{std::move(name)}
  , m_type{type}
  , m_shaderId{shaderId}
{
  assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
  assert(m_shaderId != 0);
}

Shader::Shader(Shader&& other) noexcept
  : m_name{std::move(other.m_name)}
  , m_type{other.m_type}
  , m_shaderId{std::exchange(other.m_shaderId, 0)}
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
  m_name = std::move(other.m_name);
  m_type = other.m_type;
  m_shaderId = std::exchange(other.m_shaderId, 0);
  return *this;
}

Shader::~Shader()
{
  if (m_shaderId != 0)
  {
    glAssert(glDeleteShader(m_shaderId));
    m_shaderId = 0;
  }
}

void Shader::attach(const GLuint programId) const
{
  assert(m_shaderId != 0);
  glAssert(glAttachShader(programId, m_shaderId));
}

namespace
{

Result<std::vector<std::string>> loadSource(const std::filesystem::path& path)
{
  return IO::Disk::withInputStream(path, [](auto& stream) {
    std::string line;
    std::vector<std::string> lines;

    while (!stream.eof())
    {
      std::getline(stream, line);
      lines.push_back(line + '\n');
    }

    return lines;
  });
}

std::string getInfoLog(const GLuint shaderId)
{
  auto infoLogLength = GLint{};
  glAssert(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
  if (infoLogLength > 0)
  {
    auto infoLog = std::string{};
    infoLog.resize(size_t(infoLogLength));

    glAssert(glGetShaderInfoLog(shaderId, infoLogLength, &infoLogLength, infoLog.data()));
    return infoLog;
  }

  return "Unknown error";
}

} // namespace

Result<Shader> loadShader(const std::filesystem::path& path, const GLenum type)
{
  auto name = path.filename().string();
  auto shaderId = GLuint{0};
  glAssert(shaderId = glCreateShader(type));

  if (shaderId == 0)
  {
    return Error{"Could not create shader " + name};
  }

  return loadSource(path) | kdl::and_then([&](const auto& source) -> Result<Shader> {
           const auto linePtrs =
             kdl::vec_transform(source, [](const auto& line) { return line.c_str(); });

           glAssert(glShaderSource(
             shaderId, GLsizei(linePtrs.size()), linePtrs.data(), nullptr));
           glAssert(glCompileShader(shaderId));

           auto compileStatus = GLint{};
           glAssert(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus));

           if (compileStatus == 0)
           {
             return Error{
               "Could not compile shader '" + name + "': " + getInfoLog(shaderId)};
           }

           return Shader{std::move(name), type, shaderId};
         });
}

} // namespace TrenchBroom::Renderer
