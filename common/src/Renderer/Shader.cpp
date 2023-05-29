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

#include "Exceptions.h"
#include "IO/DiskIO.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
Shader::Shader(const std::filesystem::path& path, const GLenum type)
  : m_name(path.filename().string())
  , m_type(type)
  , m_shaderId(0)
{
  assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
  glAssert(m_shaderId = glCreateShader(m_type));

  if (m_shaderId == 0)
    throw RenderException("Could not create shader " + m_name);

  const std::vector<std::string> source = loadSource(path);
  const char** linePtrs = new const char*[source.size()];
  for (size_t i = 0; i < source.size(); i++)
    linePtrs[i] = source[i].c_str();

  glAssert(
    glShaderSource(m_shaderId, static_cast<GLsizei>(source.size()), linePtrs, nullptr));
  delete[] linePtrs;

  glAssert(glCompileShader(m_shaderId));
  GLint compileStatus;
  glAssert(glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &compileStatus));

  if (compileStatus == 0)
  {
    auto str = std::stringstream();
    str << "Could not compile shader " << m_name << ": ";

    GLint infoLogLength;
    glAssert(glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0)
    {
      char* infoLog = new char[static_cast<size_t>(infoLogLength)];
      glAssert(glGetShaderInfoLog(m_shaderId, infoLogLength, &infoLogLength, infoLog));
      infoLog[infoLogLength - 1] = 0;

      str << infoLog;
      delete[] infoLog;
    }
    else
    {
      str << "Unknown error";
    }

    throw RenderException(str.str());
  }
}

Shader::~Shader()
{
  if (m_shaderId != 0)
  {
    glAssert(glDeleteShader(m_shaderId));
    m_shaderId = 0;
  }
}

void Shader::attach(const GLuint programId)
{
  glAssert(glAttachShader(programId, m_shaderId));
}

void Shader::detach(const GLuint programId)
{
  glAssert(glDetachShader(programId, m_shaderId));
}

std::vector<std::string> Shader::loadSource(const std::filesystem::path& path)
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
} // namespace Renderer
} // namespace TrenchBroom
