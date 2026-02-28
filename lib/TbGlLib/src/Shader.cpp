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

#include "gl/Shader.h"

#include "fs/DiskIO.h"
#include "gl/GlInterface.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/result.h"

#include <ranges>
#include <string>
#include <vector>

namespace tb::gl
{

Shader::Shader(std::string name, const GLenum type, const GLuint shaderId)
  : m_name{std::move(name)}
  , m_type{type}
  , m_shaderId{shaderId}
{
  contract_pre(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
  contract_pre(m_shaderId != 0);
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

void Shader::attach(Gl& gl, const GLuint programId) const
{
  contract_pre(m_shaderId != 0);

  gl.attachShader(programId, m_shaderId);
}

void Shader::destroy(Gl& gl)
{
  if (m_shaderId != 0)
  {
    gl.deleteShader(m_shaderId);
    m_shaderId = 0;
  }
}

namespace
{

Result<std::vector<std::string>> loadSource(const std::filesystem::path& path)
{
  return fs::Disk::withInputStream(path, [](auto& stream) {
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

std::string getInfoLog(Gl& gl, const GLuint shaderId)
{
  auto infoLogLength = GLint{};
  gl.getShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0)
  {
    auto infoLog = std::string{};
    infoLog.resize(size_t(infoLogLength));

    gl.getShaderInfoLog(shaderId, infoLogLength, &infoLogLength, infoLog.data());
    return infoLog;
  }

  return "Unknown error";
}

} // namespace

Result<Shader> loadShader(Gl& gl, const std::filesystem::path& path, const GLenum type)
{
  auto name = path.filename().string();
  const auto shaderId = gl.createShader(type);

  if (shaderId == 0)
  {
    return Error{"Could not create shader " + name};
  }

  return loadSource(path) | kdl::and_then([&](const auto& source) -> Result<Shader> {
           const auto linePtrs =
             source | std::views::transform([](const auto& line) { return line.c_str(); })
             | kdl::ranges::to<std::vector>();

           gl.shaderSource(shaderId, GLsizei(linePtrs.size()), linePtrs.data(), nullptr);
           gl.compileShader(shaderId);

           auto compileStatus = GLint{};
           gl.getShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

           if (compileStatus == 0)
           {
             return Error{
               "Could not compile shader '" + name + "': " + getInfoLog(gl, shaderId)};
           }

           return Shader{std::move(name), type, shaderId};
         });
}

} // namespace tb::gl
