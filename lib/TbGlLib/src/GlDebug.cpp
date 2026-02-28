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

#include "gl/GlDebug.h"

#include <fmt/format.h>

namespace tb::gl
{
namespace
{

std::string getErrorMessage(const GLenum code)
{
  switch (code)
  {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_STACK_OVERFLOW:
    return "GL_STACK_OVERFLOW";
  case GL_STACK_UNDERFLOW:
    return "GL_STACK_UNDERFLOW";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
#ifndef __APPLE__
  case GL_CONTEXT_LOST:
    return "GL_CONTEXT_LOST";
#endif
  case GL_TABLE_TOO_LARGE:
    return "GL_TABLE_TOO_LARGE";
  default:
    return "UNKNOWN";
  }
}

void checkError(Gl& gl, const std::string& msg)
{
  const GLenum error = gl.getError();
  if (error != GL_NO_ERROR)
  {
    throw std::runtime_error{
      fmt::format("OpenGL error: {} ({}) {}", error, getErrorMessage(error), msg)};
  }
}

} // namespace

// #define GL_LOG 1
#if defined(GL_LOG)
#define glAssert(C)                                                                      \
  do                                                                                     \
  {                                                                                      \
    std::cout << #C << std::endl;                                                        \
    checkError(m_gl, "before " #C);                                                      \
    (C);                                                                                 \
    checkError(m_gl, "after " #C);                                                       \
  } while (0)
#else
#define glAssert(C)                                                                      \
  do                                                                                     \
  {                                                                                      \
    checkError(m_gl, "before " #C);                                                      \
    (C);                                                                                 \
    checkError(m_gl, "after " #C);                                                       \
  } while (0)
#endif


GlDebug::GlDebug(Gl& gl)
  : m_gl{gl}
{
}

void GlDebug::clear(const GLbitfield mask)
{
  glAssert(m_gl.clear(mask));
}

void GlDebug::clearColor(
  const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha)
{
  glAssert(m_gl.clearColor(red, green, blue, alpha));
}

void GlDebug::viewport(
  const GLint x, const GLint y, const GLsizei width, const GLsizei height)
{
  glAssert(m_gl.viewport(x, y, width, height));
}

void GlDebug::matrixMode(const GLenum mode)
{
  glAssert(m_gl.matrixMode(mode));
}

void GlDebug::loadMatrixd(const GLdouble* matrix)
{
  glAssert(m_gl.loadMatrixd(matrix));
}

void GlDebug::loadMatrixf(const GLfloat* matrix)
{
  glAssert(m_gl.loadMatrixf(matrix));
}

void GlDebug::getBooleanv(const GLenum pname, GLboolean* params)
{
  glAssert(m_gl.getBooleanv(pname, params));
}

void GlDebug::getDoublev(const GLenum pname, GLdouble* params)
{
  glAssert(m_gl.getDoublev(pname, params));
}

void GlDebug::getFloatv(const GLenum pname, GLfloat* params)
{
  glAssert(m_gl.getFloatv(pname, params));
}

void GlDebug::getIntegerv(const GLenum pname, GLint* params)
{
  glAssert(m_gl.getIntegerv(pname, params));
}

void GlDebug::enableClientState(const GLenum cap)
{
  glAssert(m_gl.enableClientState(cap));
}

void GlDebug::disableClientState(const GLenum cap)
{
  glAssert(m_gl.disableClientState(cap));
}

void GlDebug::pushAttrib(const GLbitfield mask)
{
  glAssert(m_gl.pushAttrib(mask));
}

void GlDebug::popAttrib()
{
  glAssert(m_gl.popAttrib());
}

void GlDebug::enable(const GLenum cap)
{
  glAssert(m_gl.enable(cap));
}

void GlDebug::disable(const GLenum cap)
{
  glAssert(m_gl.disable(cap));
}

void GlDebug::lineWidth(const GLfloat width)
{
  glAssert(m_gl.lineWidth(width));
}

void GlDebug::polygonMode(const GLenum face, const GLenum mode)
{
  glAssert(m_gl.polygonMode(face, mode));
}

void GlDebug::frontFace(const GLenum mode)
{
  glAssert(m_gl.frontFace(mode));
}

void GlDebug::cullFace(const GLenum mode)
{
  glAssert(m_gl.cullFace(mode));
}

void GlDebug::blendFunc(const GLenum sfactor, const GLenum dfactor)
{
  glAssert(m_gl.blendFunc(sfactor, dfactor));
}

void GlDebug::shadeModel(const GLenum mode)
{
  glAssert(m_gl.shadeModel(mode));
}

void GlDebug::depthMask(const GLboolean flag)
{
  glAssert(m_gl.depthMask(flag));
}

void GlDebug::depthRange(const GLclampd nearVal, const GLclampd farVal)
{
  glAssert(m_gl.depthRange(nearVal, farVal));
}

void GlDebug::depthFunc(const GLenum func)
{
  glAssert(m_gl.depthFunc(func));
}

GLuint GlDebug::createProgram()
{
  auto result = GLuint{0};
  glAssert(result = m_gl.createProgram());
  return result;
}

void GlDebug::deleteProgram(const GLuint program)
{
  glAssert(m_gl.deleteProgram(program));
}

void GlDebug::linkProgram(const GLuint program)
{
  glAssert(m_gl.linkProgram(program));
}

void GlDebug::getProgramInfoLog(
  const GLuint program, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  glAssert(m_gl.getProgramInfoLog(program, maxLength, length, infoLog));
}

void GlDebug::getProgramiv(const GLuint program, const GLenum pname, GLint* params)
{
  glAssert(m_gl.getProgramiv(program, pname, params));
}

void GlDebug::useProgram(const GLuint program)
{
  glAssert(m_gl.useProgram(program));
}

GLuint GlDebug::createShader(const GLenum shaderType)
{
  auto result = GLuint{0};
  glAssert(result = m_gl.createShader(shaderType));
  return result;
}

void GlDebug::deleteShader(const GLuint shader)
{
  glAssert(m_gl.deleteShader(shader));
}

void GlDebug::attachShader(const GLuint program, const GLuint shader)
{
  glAssert(m_gl.attachShader(program, shader));
}

void GlDebug::shaderSource(
  const GLuint shader,
  const GLsizei count,
  const GLchar* const* string,
  const GLint* length)
{
  glAssert(m_gl.shaderSource(shader, count, string, length));
}

void GlDebug::compileShader(const GLuint shader)
{
  glAssert(m_gl.compileShader(shader));
}

void GlDebug::getShaderInfoLog(
  const GLuint shader, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  glAssert(m_gl.getShaderInfoLog(shader, maxLength, length, infoLog));
}

void GlDebug::getShaderiv(const GLuint shader, const GLenum pname, GLint* params)
{
  glAssert(m_gl.getShaderiv(shader, pname, params));
}

void GlDebug::uniform1f(const GLint location, const GLfloat v0)
{
  glAssert(m_gl.uniform1f(location, v0));
}

void GlDebug::uniform2f(const GLint location, const GLfloat v0, const GLfloat v1)
{
  glAssert(m_gl.uniform2f(location, v0, v1));
}

void GlDebug::uniform3f(
  const GLint location, const GLfloat v0, const GLfloat v1, const GLfloat v2)
{
  glAssert(m_gl.uniform3f(location, v0, v1, v2));
}

void GlDebug::uniform4f(
  const GLint location,
  const GLfloat v0,
  const GLfloat v1,
  const GLfloat v2,
  const GLfloat v3)
{
  glAssert(m_gl.uniform4f(location, v0, v1, v2, v3));
}

void GlDebug::uniform1i(const GLint location, const GLint v0)
{
  glAssert(m_gl.uniform1i(location, v0));
}

void GlDebug::uniform2i(const GLint location, const GLint v0, const GLint v1)
{
  glAssert(m_gl.uniform2i(location, v0, v1));
}

void GlDebug::uniform3i(
  const GLint location, const GLint v0, const GLint v1, const GLint v2)
{
  glAssert(m_gl.uniform3i(location, v0, v1, v2));
}

void GlDebug::uniform4i(
  const GLint location, const GLint v0, const GLint v1, const GLint v2, const GLint v3)
{
  glAssert(m_gl.uniform4i(location, v0, v1, v2, v3));
}

void GlDebug::uniform1fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glAssert(m_gl.uniform1fv(location, count, value));
}

void GlDebug::uniform2fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glAssert(m_gl.uniform2fv(location, count, value));
}

void GlDebug::uniform3fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glAssert(m_gl.uniform3fv(location, count, value));
}

void GlDebug::uniform4fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glAssert(m_gl.uniform4fv(location, count, value));
}

void GlDebug::uniform1iv(const GLint location, const GLsizei count, const GLint* value)
{
  glAssert(m_gl.uniform1iv(location, count, value));
}

void GlDebug::uniform2iv(const GLint location, const GLsizei count, const GLint* value)
{
  glAssert(m_gl.uniform2iv(location, count, value));
}

void GlDebug::uniform3iv(const GLint location, const GLsizei count, const GLint* value)
{
  glAssert(m_gl.uniform3iv(location, count, value));
}

void GlDebug::uniform4iv(const GLint location, const GLsizei count, const GLint* value)
{
  glAssert(m_gl.uniform4iv(location, count, value));
}

void GlDebug::uniformMatrix2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix2fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix3fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix4fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix2x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix2x3fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix3x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix3x2fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix2x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix2x4fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix4x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix4x2fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix3x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix3x4fv(location, count, transpose, value));
}

void GlDebug::uniformMatrix4x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glAssert(m_gl.uniformMatrix4x3fv(location, count, transpose, value));
}

GLint GlDebug::getAttribLocation(const GLuint program, const GLchar* name)
{
  auto result = GLint{-1};
  glAssert(result = m_gl.getAttribLocation(program, name));
  return result;
}

GLint GlDebug::getUniformLocation(const GLuint program, const GLchar* name)
{
  auto result = GLint{-1};
  glAssert(result = m_gl.getUniformLocation(program, name));
  return result;
}

void GlDebug::genBuffers(const GLsizei n, GLuint* buffers)
{
  glAssert(m_gl.genBuffers(n, buffers));
}

void GlDebug::deleteBuffers(const GLsizei n, const GLuint* buffers)
{
  glAssert(m_gl.deleteBuffers(n, buffers));
}

void GlDebug::bindBuffer(const GLenum target, const GLuint buffer)
{
  glAssert(m_gl.bindBuffer(target, buffer));
}

void GlDebug::bufferData(
  const GLenum target, const GLsizeiptr size, const GLvoid* data, const GLenum usage)
{
  glAssert(m_gl.bufferData(target, size, data, usage));
}

void GlDebug::bufferSubData(
  const GLenum target, const GLintptr offset, const GLsizeiptr size, const void* data)
{
  glAssert(m_gl.bufferSubData(target, offset, size, data));
}

void GlDebug::vertexPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  glAssert(m_gl.vertexPointer(size, type, stride, ptr));
}

void GlDebug::colorPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  glAssert(m_gl.colorPointer(size, type, stride, pointer));
}

void GlDebug::normalPointer(const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  glAssert(m_gl.normalPointer(type, stride, ptr));
}

void GlDebug::texCoordPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  glAssert(m_gl.texCoordPointer(size, type, stride, pointer));
}

void GlDebug::enableVertexAttribArray(const GLuint index)
{
  glAssert(m_gl.enableVertexAttribArray(index));
}

void GlDebug::disableVertexAttribArray(const GLuint index)
{
  glAssert(m_gl.disableVertexAttribArray(index));
}

void GlDebug::vertexAttribPointer(
  const GLuint index,
  const GLint size,
  const GLenum type,
  const GLboolean normalized,
  const GLsizei stride,
  const void* pointer)
{
  glAssert(m_gl.vertexAttribPointer(index, size, type, normalized, stride, pointer));
}

void GlDebug::genTextures(const GLsizei n, GLuint* textures)
{
  glAssert(m_gl.genTextures(n, textures));
}

void GlDebug::deleteTextures(const GLsizei n, const GLuint* textures)
{
  glAssert(m_gl.deleteTextures(n, textures));
}

void GlDebug::bindTexture(const GLenum target, const GLuint texture)
{
  glAssert(m_gl.bindTexture(target, texture));
}

void GlDebug::activeTexture(const GLenum texture)
{
  glAssert(m_gl.activeTexture(texture));
}

void GlDebug::texImage2D(
  const GLenum target,
  const GLint level,
  const GLint internalFormat,
  const GLsizei width,
  const GLsizei height,
  const GLint border,
  const GLenum format,
  const GLenum type,
  const GLvoid* data)
{
  glAssert(m_gl.texImage2D(
    target, level, internalFormat, width, height, border, format, type, data));
}

void GlDebug::compressedTexImage2D(
  const GLenum target,
  const GLint level,
  const GLenum internalformat,
  const GLsizei width,
  const GLsizei height,
  const GLint border,
  const GLsizei imageSize,
  const GLvoid* data)
{
  glAssert(m_gl.compressedTexImage2D(
    target, level, internalformat, width, height, border, imageSize, data));
}

void GlDebug::texParameterf(const GLenum target, const GLenum pname, const GLfloat param)
{
  glAssert(m_gl.texParameterf(target, pname, param));
}

void GlDebug::texParameteri(const GLenum target, const GLenum pname, const GLint param)
{
  glAssert(m_gl.texParameteri(target, pname, param));
}

void GlDebug::pixelStoref(const GLenum pname, const GLfloat param)
{
  glAssert(m_gl.pixelStoref(pname, param));
}

void GlDebug::pixelStorei(const GLenum pname, const GLint param)
{
  glAssert(m_gl.pixelStorei(pname, param));
}

void GlDebug::clientActiveTexture(const GLenum texture)
{
  glAssert(m_gl.clientActiveTexture(texture));
}

void GlDebug::drawArrays(const GLenum mode, const GLint first, const GLsizei count)
{
  glAssert(m_gl.drawArrays(mode, first, count));
}

void GlDebug::drawElements(
  const GLenum mode, const GLsizei count, const GLenum type, const void* indices)
{
  glAssert(m_gl.drawElements(mode, count, type, indices));
}

void GlDebug::multiDrawArrays(
  const GLenum mode, const GLint* first, const GLsizei* count, const GLsizei primcount)
{
  glAssert(m_gl.multiDrawArrays(mode, first, count, primcount));
}

const GLubyte* GlDebug::getString(const GLenum name)
{
  const GLubyte* result = nullptr;
  glAssert(result = m_gl.getString(name));
  return result;
}

GLenum GlDebug::getError()
{
  return m_gl.getError();
}

} // namespace tb::gl
