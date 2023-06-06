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

#include "kdl/vector_utils.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom::Renderer
{

namespace
{
std::string getLinkErrorMessage(const std::string& name, const GLuint shaderId)
{
  auto str = std::stringstream{};
  str << "Could not compile shader " << name << ": ";

  auto infoLogLength = GLint{};
  glAssert(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
  if (infoLogLength > 0)
  {
    auto infoLog = std::string{};
    infoLog.resize(size_t(infoLogLength));

    glAssert(glGetShaderInfoLog(shaderId, infoLogLength, &infoLogLength, infoLog.data()));
    str << infoLog;
  }
  else
  {
    str << "Unknown error";
  }

  return str.str();
}
} // namespace

Shader::Shader(const std::filesystem::path& path, const GLenum type)
  : m_name{path.filename().string()}
  , m_type{type}
{
  assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
  glAssert(m_shaderId = glCreateShader(m_type));

  if (m_shaderId == 0)
  {
    throw RenderException{"Could not create shader " + m_name};
  }

  const auto source = loadSource(path);
  const auto linePtrs =
    kdl::vec_transform(source, [](const auto& line) { return line.c_str(); });

  glAssert(
    glShaderSource(m_shaderId, GLsizei(linePtrs.size()), linePtrs.data(), nullptr));

  glAssert(glCompileShader(m_shaderId));
  auto compileStatus = GLint{};
  glAssert(glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &compileStatus));

  if (compileStatus == 0)
  {
    throw RenderException{getLinkErrorMessage(m_name, m_shaderId)};
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

void Shader::attach(const GLuint programId) const
{
  glAssert(glAttachShader(programId, m_shaderId));
}

void Shader::detach(const GLuint programId) const
{
  glAssert(glDetachShader(programId, m_shaderId));
}

std::vector<std::string> Shader::loadSource(const std::filesystem::path& path)
{
  return IO::Disk::withInputStream(
           path,
           [](auto& stream) {
             std::string line;
             std::vector<std::string> lines;

             while (!stream.eof())
             {
               std::getline(stream, line);
               lines.push_back(line + '\n');
             }

             return lines;
           })
    .if_error([](const auto& e) { throw FileSystemException{e.msg.c_str()}; })
    .value();
}

} // namespace TrenchBroom::Renderer
