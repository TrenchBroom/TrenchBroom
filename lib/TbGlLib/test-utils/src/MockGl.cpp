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

#include "gl/MockGl.h"

#include <fmt/format.h>

#include <cctype>

namespace tb::gl
{

namespace
{

std::string capitalize(const std::string_view s)
{
  auto result = std::string{s};
  if (!result.empty())
  {
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
  }
  return result;
}

} // namespace

MockGlUnexpectedCall::MockGlUnexpectedCall(const std::string_view function)
  : std::runtime_error{fmt::format(
      "MockGl::{} called with no on{} slot assigned", function, capitalize(function))}
{
}

void MockGl::clear(const GLbitfield mask)
{
  if (!onClear)
  {
    throw MockGlUnexpectedCall{"clear"};
  }
  onClear(mask);
}

void MockGl::clearColor(
  const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha)
{
  if (!onClearColor)
  {
    throw MockGlUnexpectedCall{"clearColor"};
  }
  onClearColor(red, green, blue, alpha);
}

void MockGl::viewport(
  const GLint x, const GLint y, const GLsizei width, const GLsizei height)
{
  if (!onViewport)
  {
    throw MockGlUnexpectedCall{"viewport"};
  }
  onViewport(x, y, width, height);
}

void MockGl::matrixMode(const GLenum mode)
{
  if (!onMatrixMode)
  {
    throw MockGlUnexpectedCall{"matrixMode"};
  }
  onMatrixMode(mode);
}

void MockGl::loadMatrixd(const GLdouble* matrix)
{
  if (!onLoadMatrixd)
  {
    throw MockGlUnexpectedCall{"loadMatrixd"};
  }
  onLoadMatrixd(matrix);
}

void MockGl::loadMatrixf(const GLfloat* matrix)
{
  if (!onLoadMatrixf)
  {
    throw MockGlUnexpectedCall{"loadMatrixf"};
  }
  onLoadMatrixf(matrix);
}

void MockGl::getBooleanv(const GLenum pname, GLboolean* params)
{
  if (!onGetBooleanv)
  {
    throw MockGlUnexpectedCall{"getBooleanv"};
  }
  onGetBooleanv(pname, params);
}

void MockGl::getDoublev(const GLenum pname, GLdouble* params)
{
  if (!onGetDoublev)
  {
    throw MockGlUnexpectedCall{"getDoublev"};
  }
  onGetDoublev(pname, params);
}

void MockGl::getFloatv(const GLenum pname, GLfloat* params)
{
  if (!onGetFloatv)
  {
    throw MockGlUnexpectedCall{"getFloatv"};
  }
  onGetFloatv(pname, params);
}

void MockGl::getIntegerv(const GLenum pname, GLint* params)
{
  if (!onGetIntegerv)
  {
    throw MockGlUnexpectedCall{"getIntegerv"};
  }
  onGetIntegerv(pname, params);
}

void MockGl::enableClientState(const GLenum cap)
{
  if (!onEnableClientState)
  {
    throw MockGlUnexpectedCall{"enableClientState"};
  }
  onEnableClientState(cap);
}

void MockGl::disableClientState(const GLenum cap)
{
  if (!onDisableClientState)
  {
    throw MockGlUnexpectedCall{"disableClientState"};
  }
  onDisableClientState(cap);
}

void MockGl::pushAttrib(const GLbitfield mask)
{
  if (!onPushAttrib)
  {
    throw MockGlUnexpectedCall{"pushAttrib"};
  }
  onPushAttrib(mask);
}

void MockGl::popAttrib()
{
  if (!onPopAttrib)
  {
    throw MockGlUnexpectedCall{"popAttrib"};
  }
  onPopAttrib();
}

void MockGl::enable(const GLenum cap)
{
  if (!onEnable)
  {
    throw MockGlUnexpectedCall{"enable"};
  }
  onEnable(cap);
}

void MockGl::disable(const GLenum cap)
{
  if (!onDisable)
  {
    throw MockGlUnexpectedCall{"disable"};
  }
  onDisable(cap);
}

void MockGl::lineWidth(const GLfloat width)
{
  if (!onLineWidth)
  {
    throw MockGlUnexpectedCall{"lineWidth"};
  }
  onLineWidth(width);
}

void MockGl::polygonMode(const GLenum face, const GLenum mode)
{
  if (!onPolygonMode)
  {
    throw MockGlUnexpectedCall{"polygonMode"};
  }
  onPolygonMode(face, mode);
}

void MockGl::frontFace(const GLenum mode)
{
  if (!onFrontFace)
  {
    throw MockGlUnexpectedCall{"frontFace"};
  }
  onFrontFace(mode);
}

void MockGl::cullFace(const GLenum mode)
{
  if (!onCullFace)
  {
    throw MockGlUnexpectedCall{"cullFace"};
  }
  onCullFace(mode);
}

void MockGl::blendFunc(const GLenum sfactor, const GLenum dfactor)
{
  if (!onBlendFunc)
  {
    throw MockGlUnexpectedCall{"blendFunc"};
  }
  onBlendFunc(sfactor, dfactor);
}

void MockGl::shadeModel(const GLenum mode)
{
  if (!onShadeModel)
  {
    throw MockGlUnexpectedCall{"shadeModel"};
  }
  onShadeModel(mode);
}

void MockGl::depthMask(const GLboolean flag)
{
  if (!onDepthMask)
  {
    throw MockGlUnexpectedCall{"depthMask"};
  }
  onDepthMask(flag);
}

void MockGl::depthRange(const GLclampd nearVal, const GLclampd farVal)
{
  if (!onDepthRange)
  {
    throw MockGlUnexpectedCall{"depthRange"};
  }
  onDepthRange(nearVal, farVal);
}

void MockGl::colorMask(
  const GLboolean red, const GLboolean green, const GLboolean blue, const GLboolean alpha)
{
  if (!onColorMask)
  {
    throw MockGlUnexpectedCall{"colorMask"};
  }
  onColorMask(red, green, blue, alpha);
}

void MockGl::depthFunc(const GLenum func)
{
  if (!onDepthFunc)
  {
    throw MockGlUnexpectedCall{"depthFunc"};
  }
  onDepthFunc(func);
}

GLuint MockGl::createProgram()
{
  if (!onCreateProgram)
  {
    throw MockGlUnexpectedCall{"createProgram"};
  }
  return onCreateProgram();
}

void MockGl::deleteProgram(const GLuint program)
{
  if (!onDeleteProgram)
  {
    throw MockGlUnexpectedCall{"deleteProgram"};
  }
  onDeleteProgram(program);
}

void MockGl::linkProgram(const GLuint program)
{
  if (!onLinkProgram)
  {
    throw MockGlUnexpectedCall{"linkProgram"};
  }
  onLinkProgram(program);
}

void MockGl::getProgramInfoLog(
  const GLuint program, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  if (!onGetProgramInfoLog)
  {
    throw MockGlUnexpectedCall{"getProgramInfoLog"};
  }
  onGetProgramInfoLog(program, maxLength, length, infoLog);
}

void MockGl::getProgramiv(const GLuint program, const GLenum pname, GLint* params)
{
  if (!onGetProgramiv)
  {
    throw MockGlUnexpectedCall{"getProgramiv"};
  }
  onGetProgramiv(program, pname, params);
}

void MockGl::useProgram(const GLuint program)
{
  if (!onUseProgram)
  {
    throw MockGlUnexpectedCall{"useProgram"};
  }
  onUseProgram(program);
}

GLuint MockGl::createShader(const GLenum shaderType)
{
  if (!onCreateShader)
  {
    throw MockGlUnexpectedCall{"createShader"};
  }
  return onCreateShader(shaderType);
}

void MockGl::deleteShader(const GLuint shader)
{
  if (!onDeleteShader)
  {
    throw MockGlUnexpectedCall{"deleteShader"};
  }
  onDeleteShader(shader);
}

void MockGl::attachShader(const GLuint program, const GLuint shader)
{
  if (!onAttachShader)
  {
    throw MockGlUnexpectedCall{"attachShader"};
  }
  onAttachShader(program, shader);
}

void MockGl::shaderSource(
  const GLuint shader,
  const GLsizei count,
  const GLchar* const* string,
  const GLint* length)
{
  if (!onShaderSource)
  {
    throw MockGlUnexpectedCall{"shaderSource"};
  }
  onShaderSource(shader, count, string, length);
}

void MockGl::compileShader(const GLuint shader)
{
  if (!onCompileShader)
  {
    throw MockGlUnexpectedCall{"compileShader"};
  }
  onCompileShader(shader);
}

void MockGl::getShaderInfoLog(
  const GLuint shader, const GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
  if (!onGetShaderInfoLog)
  {
    throw MockGlUnexpectedCall{"getShaderInfoLog"};
  }
  onGetShaderInfoLog(shader, maxLength, length, infoLog);
}

void MockGl::getShaderiv(const GLuint shader, const GLenum pname, GLint* params)
{
  if (!onGetShaderiv)
  {
    throw MockGlUnexpectedCall{"getShaderiv"};
  }
  onGetShaderiv(shader, pname, params);
}

void MockGl::uniform1f(const GLint location, const GLfloat v0)
{
  if (!onUniform1f)
  {
    throw MockGlUnexpectedCall{"uniform1f"};
  }
  onUniform1f(location, v0);
}

void MockGl::uniform2f(const GLint location, const GLfloat v0, const GLfloat v1)
{
  if (!onUniform2f)
  {
    throw MockGlUnexpectedCall{"uniform2f"};
  }
  onUniform2f(location, v0, v1);
}

void MockGl::uniform3f(
  const GLint location, const GLfloat v0, const GLfloat v1, const GLfloat v2)
{
  if (!onUniform3f)
  {
    throw MockGlUnexpectedCall{"uniform3f"};
  }
  onUniform3f(location, v0, v1, v2);
}

void MockGl::uniform4f(
  const GLint location,
  const GLfloat v0,
  const GLfloat v1,
  const GLfloat v2,
  const GLfloat v3)
{
  if (!onUniform4f)
  {
    throw MockGlUnexpectedCall{"uniform4f"};
  }
  onUniform4f(location, v0, v1, v2, v3);
}

void MockGl::uniform1i(const GLint location, const GLint v0)
{
  if (!onUniform1i)
  {
    throw MockGlUnexpectedCall{"uniform1i"};
  }
  onUniform1i(location, v0);
}

void MockGl::uniform2i(const GLint location, const GLint v0, const GLint v1)
{
  if (!onUniform2i)
  {
    throw MockGlUnexpectedCall{"uniform2i"};
  }
  onUniform2i(location, v0, v1);
}

void MockGl::uniform3i(
  const GLint location, const GLint v0, const GLint v1, const GLint v2)
{
  if (!onUniform3i)
  {
    throw MockGlUnexpectedCall{"uniform3i"};
  }
  onUniform3i(location, v0, v1, v2);
}

void MockGl::uniform4i(
  const GLint location, const GLint v0, const GLint v1, const GLint v2, const GLint v3)
{
  if (!onUniform4i)
  {
    throw MockGlUnexpectedCall{"uniform4i"};
  }
  onUniform4i(location, v0, v1, v2, v3);
}

void MockGl::uniform1fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  if (!onUniform1fv)
  {
    throw MockGlUnexpectedCall{"uniform1fv"};
  }
  onUniform1fv(location, count, value);
}

void MockGl::uniform2fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  if (!onUniform2fv)
  {
    throw MockGlUnexpectedCall{"uniform2fv"};
  }
  onUniform2fv(location, count, value);
}

void MockGl::uniform3fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  if (!onUniform3fv)
  {
    throw MockGlUnexpectedCall{"uniform3fv"};
  }
  onUniform3fv(location, count, value);
}

void MockGl::uniform4fv(const GLint location, const GLsizei count, const GLfloat* value)
{
  if (!onUniform4fv)
  {
    throw MockGlUnexpectedCall{"uniform4fv"};
  }
  onUniform4fv(location, count, value);
}

void MockGl::uniform1iv(const GLint location, const GLsizei count, const GLint* value)
{
  if (!onUniform1iv)
  {
    throw MockGlUnexpectedCall{"uniform1iv"};
  }
  onUniform1iv(location, count, value);
}

void MockGl::uniform2iv(const GLint location, const GLsizei count, const GLint* value)
{
  if (!onUniform2iv)
  {
    throw MockGlUnexpectedCall{"uniform2iv"};
  }
  onUniform2iv(location, count, value);
}

void MockGl::uniform3iv(const GLint location, const GLsizei count, const GLint* value)
{
  if (!onUniform3iv)
  {
    throw MockGlUnexpectedCall{"uniform3iv"};
  }
  onUniform3iv(location, count, value);
}

void MockGl::uniform4iv(const GLint location, const GLsizei count, const GLint* value)
{
  if (!onUniform4iv)
  {
    throw MockGlUnexpectedCall{"uniform4iv"};
  }
  onUniform4iv(location, count, value);
}

void MockGl::uniformMatrix2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix2fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix2fv"};
  }
  onUniformMatrix2fv(location, count, transpose, value);
}

void MockGl::uniformMatrix3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix3fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix3fv"};
  }
  onUniformMatrix3fv(location, count, transpose, value);
}

void MockGl::uniformMatrix4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix4fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix4fv"};
  }
  onUniformMatrix4fv(location, count, transpose, value);
}

void MockGl::uniformMatrix2x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix2x3fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix2x3fv"};
  }
  onUniformMatrix2x3fv(location, count, transpose, value);
}

void MockGl::uniformMatrix3x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix3x2fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix3x2fv"};
  }
  onUniformMatrix3x2fv(location, count, transpose, value);
}

void MockGl::uniformMatrix2x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix2x4fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix2x4fv"};
  }
  onUniformMatrix2x4fv(location, count, transpose, value);
}

void MockGl::uniformMatrix4x2fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix4x2fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix4x2fv"};
  }
  onUniformMatrix4x2fv(location, count, transpose, value);
}

void MockGl::uniformMatrix3x4fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix3x4fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix3x4fv"};
  }
  onUniformMatrix3x4fv(location, count, transpose, value);
}

void MockGl::uniformMatrix4x3fv(
  const GLint location,
  const GLsizei count,
  const GLboolean transpose,
  const GLfloat* value)
{
  if (!onUniformMatrix4x3fv)
  {
    throw MockGlUnexpectedCall{"uniformMatrix4x3fv"};
  }
  onUniformMatrix4x3fv(location, count, transpose, value);
}

GLint MockGl::getAttribLocation(const GLuint program, const GLchar* name)
{
  if (!onGetAttribLocation)
  {
    throw MockGlUnexpectedCall{"getAttribLocation"};
  }
  return onGetAttribLocation(program, name);
}

GLint MockGl::getUniformLocation(const GLuint program, const GLchar* name)
{
  if (!onGetUniformLocation)
  {
    throw MockGlUnexpectedCall{"getUniformLocation"};
  }
  return onGetUniformLocation(program, name);
}

void MockGl::genBuffers(const GLsizei n, GLuint* buffers)
{
  if (!onGenBuffers)
  {
    throw MockGlUnexpectedCall{"genBuffers"};
  }
  onGenBuffers(n, buffers);
}

void MockGl::deleteBuffers(const GLsizei n, const GLuint* buffers)
{
  if (!onDeleteBuffers)
  {
    throw MockGlUnexpectedCall{"deleteBuffers"};
  }
  onDeleteBuffers(n, buffers);
}

void MockGl::bindBuffer(const GLenum target, const GLuint buffer)
{
  if (!onBindBuffer)
  {
    throw MockGlUnexpectedCall{"bindBuffer"};
  }
  onBindBuffer(target, buffer);
}

void MockGl::bufferData(
  const GLenum target, const GLsizeiptr size, const GLvoid* data, const GLenum usage)
{
  if (!onBufferData)
  {
    throw MockGlUnexpectedCall{"bufferData"};
  }
  onBufferData(target, size, data, usage);
}

void MockGl::bufferSubData(
  const GLenum target, const GLintptr offset, const GLsizeiptr size, const void* data)
{
  if (!onBufferSubData)
  {
    throw MockGlUnexpectedCall{"bufferSubData"};
  }
  onBufferSubData(target, offset, size, data);
}

void MockGl::vertexPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  if (!onVertexPointer)
  {
    throw MockGlUnexpectedCall{"vertexPointer"};
  }
  onVertexPointer(size, type, stride, ptr);
}

void MockGl::colorPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  if (!onColorPointer)
  {
    throw MockGlUnexpectedCall{"colorPointer"};
  }
  onColorPointer(size, type, stride, pointer);
}

void MockGl::normalPointer(const GLenum type, const GLsizei stride, const GLvoid* ptr)
{
  if (!onNormalPointer)
  {
    throw MockGlUnexpectedCall{"normalPointer"};
  }
  onNormalPointer(type, stride, ptr);
}

void MockGl::texCoordPointer(
  const GLint size, const GLenum type, const GLsizei stride, const GLvoid* pointer)
{
  if (!onTexCoordPointer)
  {
    throw MockGlUnexpectedCall{"texCoordPointer"};
  }
  onTexCoordPointer(size, type, stride, pointer);
}

void MockGl::enableVertexAttribArray(const GLuint index)
{
  if (!onEnableVertexAttribArray)
  {
    throw MockGlUnexpectedCall{"enableVertexAttribArray"};
  }
  onEnableVertexAttribArray(index);
}

void MockGl::disableVertexAttribArray(const GLuint index)
{
  if (!onDisableVertexAttribArray)
  {
    throw MockGlUnexpectedCall{"disableVertexAttribArray"};
  }
  onDisableVertexAttribArray(index);
}

void MockGl::vertexAttribPointer(
  const GLuint index,
  const GLint size,
  const GLenum type,
  const GLboolean normalized,
  const GLsizei stride,
  const void* pointer)
{
  if (!onVertexAttribPointer)
  {
    throw MockGlUnexpectedCall{"vertexAttribPointer"};
  }
  onVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void MockGl::genTextures(const GLsizei n, GLuint* textures)
{
  if (!onGenTextures)
  {
    throw MockGlUnexpectedCall{"genTextures"};
  }
  onGenTextures(n, textures);
}

void MockGl::deleteTextures(const GLsizei n, const GLuint* textures)
{
  if (!onDeleteTextures)
  {
    throw MockGlUnexpectedCall{"deleteTextures"};
  }
  onDeleteTextures(n, textures);
}

void MockGl::bindTexture(const GLenum target, const GLuint texture)
{
  if (!onBindTexture)
  {
    throw MockGlUnexpectedCall{"bindTexture"};
  }
  onBindTexture(target, texture);
}

void MockGl::activeTexture(const GLenum texture)
{
  if (!onActiveTexture)
  {
    throw MockGlUnexpectedCall{"activeTexture"};
  }
  onActiveTexture(texture);
}

void MockGl::texImage2D(
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
  if (!onTexImage2D)
  {
    throw MockGlUnexpectedCall{"texImage2D"};
  }
  onTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
}

void MockGl::compressedTexImage2D(
  const GLenum target,
  const GLint level,
  const GLenum internalformat,
  const GLsizei width,
  const GLsizei height,
  const GLint border,
  const GLsizei imageSize,
  const GLvoid* data)
{
  if (!onCompressedTexImage2D)
  {
    throw MockGlUnexpectedCall{"compressedTexImage2D"};
  }
  onCompressedTexImage2D(
    target, level, internalformat, width, height, border, imageSize, data);
}

void MockGl::texParameterf(const GLenum target, const GLenum pname, const GLfloat param)
{
  if (!onTexParameterf)
  {
    throw MockGlUnexpectedCall{"texParameterf"};
  }
  onTexParameterf(target, pname, param);
}

void MockGl::texParameteri(const GLenum target, const GLenum pname, const GLint param)
{
  if (!onTexParameteri)
  {
    throw MockGlUnexpectedCall{"texParameteri"};
  }
  onTexParameteri(target, pname, param);
}

void MockGl::pixelStoref(const GLenum pname, const GLfloat param)
{
  if (!onPixelStoref)
  {
    throw MockGlUnexpectedCall{"pixelStoref"};
  }
  onPixelStoref(pname, param);
}

void MockGl::pixelStorei(const GLenum pname, const GLint param)
{
  if (!onPixelStorei)
  {
    throw MockGlUnexpectedCall{"pixelStorei"};
  }
  onPixelStorei(pname, param);
}

void MockGl::clientActiveTexture(const GLenum texture)
{
  if (!onClientActiveTexture)
  {
    throw MockGlUnexpectedCall{"clientActiveTexture"};
  }
  onClientActiveTexture(texture);
}

void MockGl::drawArrays(const GLenum mode, const GLint first, const GLsizei count)
{
  if (!onDrawArrays)
  {
    throw MockGlUnexpectedCall{"drawArrays"};
  }
  onDrawArrays(mode, first, count);
}

void MockGl::drawElements(
  const GLenum mode, const GLsizei count, const GLenum type, const void* indices)
{
  if (!onDrawElements)
  {
    throw MockGlUnexpectedCall{"drawElements"};
  }
  onDrawElements(mode, count, type, indices);
}

void MockGl::multiDrawArrays(
  const GLenum mode, const GLint* first, const GLsizei* count, const GLsizei primcount)
{
  if (!onMultiDrawArrays)
  {
    throw MockGlUnexpectedCall{"multiDrawArrays"};
  }
  onMultiDrawArrays(mode, first, count, primcount);
}

const GLubyte* MockGl::getString(const GLenum name)
{
  if (!onGetString)
  {
    throw MockGlUnexpectedCall{"getString"};
  }
  return onGetString(name);
}

GLenum MockGl::getError()
{
  if (!onGetError)
  {
    throw MockGlUnexpectedCall{"getError"};
  }
  return onGetError();
}

} // namespace tb::gl
