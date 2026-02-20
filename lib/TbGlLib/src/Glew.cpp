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

#include "gl/Glew.h"

namespace tb::gl
{

void Glew::clear(const GLbitfield mask)
{
  glClear(mask);
}

void Glew::clearColor(
  const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha)
{
  glClearColor(red, green, blue, alpha);
}

void Glew::viewport(
  const GLint x, const GLint y, const GLsizei width, const GLsizei height)
{
  glViewport(x, y, width, height);
}

void Glew::matrixMode(const GLenum mode)
{
  glMatrixMode(mode);
}

void Glew::loadMatrixd(const GLdouble* matrix)
{
  glLoadMatrixd(matrix);
}

void Glew::loadMatrixf(const GLfloat* matrix)
{
  glLoadMatrixf(matrix);
}

void Glew::getBooleanv(const GLenum pname, GLboolean* params)
{
  glGetBooleanv(pname, params);
}

void Glew::getDoublev(const GLenum pname, GLdouble* params)
{
  glGetDoublev(pname, params);
}

void Glew::getFloatv(const GLenum pname, GLfloat* params)
{
  glGetFloatv(pname, params);
}

void Glew::getIntegerv(const GLenum pname, GLint* params)
{
  glGetIntegerv(pname, params);
}

void Glew::enableClientState(const GLenum cap)
{
  glEnableClientState(cap);
}

void Glew::disableClientState(const GLenum cap)
{
  glDisableClientState(cap);
}

void Glew::pushAttrib(const GLbitfield mask)
{
  glPushAttrib(mask);
}

void Glew::popAttrib()
{
  glPopAttrib();
}

void Glew::enable(const GLenum cap)
{
  glEnable(cap);
}

void Glew::disable(const GLenum cap)
{
  glDisable(cap);
}

void Glew::lineWidth(const GLfloat width)
{
  glLineWidth(width);
}

void Glew::polygonMode(const GLenum face, const GLenum mode)
{
  glPolygonMode(face, mode);
}

void Glew::frontFace(const GLenum mode)
{
  glFrontFace(mode);
}

void Glew::cullFace(const GLenum mode)
{
  glCullFace(mode);
}

void Glew::blendFunc(const GLenum sfactor, const GLenum dfactor)
{
  glBlendFunc(sfactor, dfactor);
}

void Glew::shadeModel(const GLenum mode)
{
  glShadeModel(mode);
}

void Glew::depthMask(const GLboolean flag)
{
  glDepthMask(flag);
}

void Glew::depthRange(const GLclampd nearVal, const GLclampd farVal)
{
  glDepthRange(nearVal, farVal);
}

void Glew::depthFunc(const GLenum func)
{
  glDepthFunc(func);
}

GLuint Glew::createProgram()
{
  return glCreateProgram();
}

void Glew::deleteProgram(const GLuint program)
{
  glDeleteProgram(program);
}

void Glew::linkProgram(const GLuint program)
{
  glLinkProgram(program);
}

void Glew::getProgramInfoLog(
  const GLuint program, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  glGetProgramInfoLog(program, maxLength, length, infoLog);
}

void Glew::getProgramiv(const GLuint program, const GLenum pname, GLint* params)
{
  glGetProgramiv(program, pname, params);
}

void Glew::useProgram(const GLuint program)
{
  glUseProgram(program);
}

GLuint Glew::createShader(const GLenum shaderType)
{
  return glCreateShader(shaderType);
}

void Glew::deleteShader(const GLuint shader)
{
  glDeleteShader(shader);
}

void Glew::attachShader(const GLuint program, const GLuint shader)
{
  glAttachShader(program, shader);
}

void Glew::shaderSource(
  const GLuint shader, const GLsizei count, const GLchar** string, const GLint* length)
{
  glShaderSource(shader, count, string, length);
}

void Glew::compileShader(const GLuint shader)
{
  glCompileShader(shader);
}

void Glew::getShaderInfoLog(
  const GLuint shader, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  glGetShaderInfoLog(shader, maxLength, length, infoLog);
}

void Glew::getShaderiv(const GLuint shader, const GLenum pname, GLint* params)
{
  glGetShaderiv(shader, pname, params);
}

void Glew::uniform1f(const GLint location, const GLfloat v0)
{
  glUniform1f(location, v0);
}

void Glew::uniform2f(const GLint location, const GLfloat v0, const GLfloat v1)
{
  glUniform2f(location, v0, v1);
}

void Glew::uniform3f(
  const GLint location, const GLfloat v0, const GLfloat v1, const GLfloat v2)
{
  glUniform3f(location, v0, v1, v2);
}

void Glew::uniform4f(
  const GLint location,
  const GLfloat v0,
  const GLfloat v1,
  const GLfloat v2,
  const GLfloat v3)
{
  glUniform4f(location, v0, v1, v2, v3);
}

void Glew::uniform1i(const GLint location, const GLint v0)
{
  glUniform1i(location, v0);
}

void Glew::uniform2i(const GLint location, const GLint v0, const GLint v1)
{
  glUniform2i(location, v0, v1);
}

void Glew::uniform3i(const GLint location, const GLint v0, const GLint v1, const GLint v2)
{
  glUniform3i(location, v0, v1, v2);
}

void Glew::uniform4i(
  const GLint location, const GLint v0, const GLint v1, const GLint v2, const GLint v3)
{
  glUniform4i(location, v0, v1, v2, v3);
}

void Glew::uniform1ui(const GLint location, const GLuint v0)
{
  glUniform1ui(location, v0);
}

void Glew::uniform2ui(const GLint location, const GLuint v0, const GLuint v1)
{
  glUniform2ui(location, v0, v1);
}

void Glew::uniform3ui(
  const GLint location, const GLuint v0, const GLuint v1, const GLuint v2)
{
  glUniform3ui(location, v0, v1, v2);
}

void Glew::uniform4ui(
  const GLint location,
  const GLuint v0,
  const GLuint v1,
  const GLuint v2,
  const GLuint v3)
{
  glUniform4ui(location, v0, v1, v2, v3);
}

void Glew::uniform1fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glUniform1fv(location, count, value);
}

void Glew::uniform2fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glUniform2fv(location, count, value);
}

void Glew::uniform3fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glUniform3fv(location, count, value);
}

void Glew::uniform4fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  glUniform4fv(location, count, value);
}

void Glew::uniform1iv(const GLint location, const GLsizei count, const GLint* value)
{
  glUniform1iv(location, count, value);
}

void Glew::uniform2iv(const GLint location, const GLsizei count, const GLint* value)
{
  glUniform2iv(location, count, value);
}

void Glew::uniform3iv(const GLint location, const GLsizei count, const GLint* value)
{
  glUniform3iv(location, count, value);
}

void Glew::uniform4iv(const GLint location, const GLsizei count, const GLint* value)
{
  glUniform4iv(location, count, value);
}

void Glew::uniform1uiv(const GLint location, const GLsizei count, const GLuint* value)
{
  glUniform1uiv(location, count, value);
}

void Glew::uniform2uiv(const GLint location, const GLsizei count, const GLuint* value)
{
  glUniform2uiv(location, count, value);
}

void Glew::uniform3uiv(const GLint location, const GLsizei count, const GLuint* value)
{
  glUniform3uiv(location, count, value);
}

void Glew::uniform4uiv(const GLint location, const GLsizei count, const GLuint* value)
{
  glUniform4uiv(location, count, value);
}

void Glew::uniformMatrix2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix2fv(location, count, transpose, value);
}

void Glew::uniformMatrix3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix3fv(location, count, transpose, value);
}

void Glew::uniformMatrix4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix4fv(location, count, transpose, value);
}

void Glew::uniformMatrix2x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix2x3fv(location, count, transpose, value);
}

void Glew::uniformMatrix3x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix3x2fv(location, count, transpose, value);
}

void Glew::uniformMatrix2x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix2x4fv(location, count, transpose, value);
}

void Glew::uniformMatrix4x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix4x2fv(location, count, transpose, value);
}

void Glew::uniformMatrix3x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix3x4fv(location, count, transpose, value);
}

void Glew::uniformMatrix4x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  glUniformMatrix4x3fv(location, count, transpose, value);
}

GLint Glew::getAttribLocation(const GLuint program, const GLchar* name)
{
  return glGetAttribLocation(program, name);
}

void Glew::genBuffers(const GLsizei n, GLuint* buffers)
{
  glGenBuffers(n, buffers);
}

void Glew::deleteBuffers(const GLsizei n, const GLuint* buffers)
{
  glDeleteBuffers(n, buffers);
}

void Glew::bindBuffer(const GLenum target, const GLuint buffer)
{
  glBindBuffer(target, buffer);
}

void Glew::bufferData(
  const GLenum target, const GLsizeiptr size, const GLvoid* data, const GLenum usage)
{
  glBufferData(target, size, data, usage);
}

void Glew::bufferSubData(
  const GLenum target, const GLintptr offset, const GLsizeiptr size, const void* data)
{
  glBufferSubData(target, offset, size, data);
}

void Glew::vertexPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  glVertexPointer(size, type, stride, ptr);
}

void Glew::colorPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  glColorPointer(size, type, stride, pointer);
}

void Glew::normalPointer(const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  glNormalPointer(type, stride, ptr);
}

void Glew::texCoordPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  glTexCoordPointer(size, type, stride, pointer);
}

void Glew::enableVertexAttribArray(const GLuint index)
{
  glEnableVertexAttribArray(index);
}

void Glew::disableVertexAttribArray(const GLuint index)
{
  glDisableVertexAttribArray(index);
}

void Glew::vertexAttribPointer(
  const GLuint index,
  const GLint size,
  const GLenum type,
  const GLboolean normalized,
  const GLsizei stride,
  const void* pointer)
{
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void Glew::genTextures(const GLsizei n, GLuint* textures)
{
  glGenTextures(n, textures);
}

void Glew::deleteTextures(const GLsizei n, const GLuint* textures)
{
  glDeleteTextures(n, textures);
}

void Glew::bindTexture(const GLenum target, const GLuint texture)
{
  glBindTexture(target, texture);
}

void Glew::activeTexture(const GLenum texture)
{
  glActiveTexture(texture);
}

void Glew::texImage2D(
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
  glTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
}

void Glew::compressedTexImage2D(
  const GLenum target,
  const GLint level,
  const GLenum internalformat,
  const GLsizei width,
  const GLsizei height,
  const GLint border,
  const GLsizei imageSize,
  const GLvoid* data)
{
  glCompressedTexImage2D(
    target, level, internalformat, width, height, border, imageSize, data);
}

void Glew::texParameterf(const GLenum target, const GLenum pname, const GLfloat param)
{
  glTexParameterf(target, pname, param);
}

void Glew::texParameteri(const GLenum target, const GLenum pname, const GLint param)
{
  glTexParameteri(target, pname, param);
}

void Glew::pixelStoref(const GLenum pname, const GLfloat param)
{
  glPixelStoref(pname, param);
}

void Glew::pixelStorei(const GLenum pname, const GLint param)
{
  glPixelStorei(pname, param);
}

void Glew::clientActiveTexture(const GLenum texture)
{
  glClientActiveTexture(texture);
}

void Glew::drawArrays(const GLenum mode, const GLint first, const GLsizei count)
{
  glDrawArrays(mode, first, count);
}

void Glew::drawElements(
  const GLenum mode, const GLsizei count, const GLenum type, const void* indices)
{
  glDrawElements(mode, count, type, indices);
}

void Glew::multiDrawArrays(
  const GLenum mode, const GLint* first, const GLsizei* count, const GLsizei primcount)
{
  glMultiDrawArrays(mode, first, count, primcount);
}

GLenum Glew::getError()
{
  return glGetError();
}

} // namespace tb::gl
