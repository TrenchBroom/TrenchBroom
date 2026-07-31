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

#include "gl/TestUtils.h"

#include "gl/MockGl.h"
#include "gl/ResourceManager.h"

namespace tb::gl
{

void processResourcesSync(
  ResourceManager& resourceManager, const ProcessContext& processContext)
{
  while (resourceManager.needsProcessing())
  {
    resourceManager.process(
      [](auto task) {
        auto promise = std::promise<std::unique_ptr<TaskResult>>{};
        promise.set_value(task());
        return promise.get_future();
      },
      processContext);
  }
}

void installVboSupport(MockGl& gl)
{
  gl.onGenBuffers = [nextId = GLuint{1}](const GLsizei n, GLuint* buffers) mutable {
    for (auto i = GLsizei{0}; i < n; ++i)
    {
      buffers[i] = nextId++;
    }
  };
  gl.onDeleteBuffers = [](GLsizei, const GLuint*) {};
  gl.onBindBuffer = [](GLenum, GLuint) {};
  gl.onBufferData = [](GLenum, GLsizeiptr, const GLvoid*, GLenum) {};
  gl.onBufferSubData = [](GLenum, GLintptr, GLsizeiptr, const void*) {};
}

void installTextureUploadSupport(MockGl& gl)
{
  gl.onGenTextures = [nextId = GLuint{1}](const GLsizei n, GLuint* textures) mutable {
    for (auto i = GLsizei{0}; i < n; ++i)
    {
      textures[i] = nextId++;
    }
  };
  gl.onDeleteTextures = [](GLsizei, const GLuint*) {};
  gl.onBindTexture = [](GLenum, GLuint) {};
  gl.onPixelStorei = [](GLenum, GLint) {};
  gl.onTexParameteri = [](GLenum, GLenum, GLint) {};
  gl.onTexImage2D =
    [](GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) {};
  gl.onCompressedTexImage2D =
    [](GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) {};
}

void installShaderCompileSupport(MockGl& gl)
{
  gl.onCreateProgram = []() { return GLuint{1}; };
  gl.onCreateShader = [](GLenum) { return GLuint{2}; };
  gl.onShaderSource = [](GLuint, GLsizei, const GLchar* const*, const GLint*) {};
  gl.onCompileShader = [](GLuint) {};
  gl.onGetShaderiv = [](GLuint, const GLenum pname, GLint* params) {
    *params = (pname == GL_COMPILE_STATUS) ? 1 : 0;
  };
  gl.onAttachShader = [](GLuint, GLuint) {};
  gl.onLinkProgram = [](GLuint) {};
  gl.onGetProgramiv = [](GLuint, const GLenum pname, GLint* params) {
    *params = (pname == GL_LINK_STATUS) ? 1 : 0;
  };
}

} // namespace tb::gl