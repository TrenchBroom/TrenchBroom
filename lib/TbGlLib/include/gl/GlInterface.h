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

#pragma once

#include <OpenGL/gl.h>

namespace tb::gl
{

class Gl
{
public:
  virtual ~Gl();

  virtual void clear(GLbitfield mask) = 0;
  virtual void clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = 0;

  virtual void viewport(GLint x, GLint y, GLsizei width, GLsizei height) = 0;

  virtual void matrixMode(GLenum mode) = 0;
  virtual void loadMatrixd(const GLdouble* matrix) = 0;
  virtual void loadMatrixf(const GLfloat* matrix) = 0;

  virtual void getBooleanv(GLenum pname, GLboolean* params) = 0;
  virtual void getDoublev(GLenum pname, GLdouble* params) = 0;
  virtual void getFloatv(GLenum pname, GLfloat* params) = 0;
  virtual void getIntegerv(GLenum pname, GLint* params) = 0;

  virtual void enableClientState(GLenum cap) = 0;
  virtual void disableClientState(GLenum cap) = 0;

  virtual void pushAttrib(GLbitfield mask) = 0;
  virtual void popAttrib() = 0;

  virtual void enable(GLenum cap) = 0;
  virtual void disable(GLenum cap) = 0;

  virtual void lineWidth(GLfloat width) = 0;

  virtual void polygonMode(GLenum face, GLenum mode) = 0;

  virtual void frontFace(GLenum mode) = 0;
  virtual void cullFace(GLenum mode) = 0;

  virtual void blendFunc(GLenum sfactor, GLenum dfactor) = 0;

  virtual void shadeModel(GLenum mode) = 0;

  virtual void depthMask(GLboolean flag) = 0;
  virtual void depthRange(GLclampd nearVal, GLclampd farVal) = 0;
  virtual void depthFunc(GLenum func) = 0;

  virtual GLuint createProgram() = 0;
  virtual void deleteProgram(GLuint program) = 0;

  virtual void linkProgram(GLuint program) = 0;

  virtual void getProgramInfoLog(
    GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog) = 0;

  virtual void getProgramiv(GLuint program, GLenum pname, GLint* params) = 0;

  virtual void useProgram(GLuint program) = 0;

  virtual GLuint createShader(GLenum shaderType) = 0;
  virtual void deleteShader(GLuint shader) = 0;

  virtual void attachShader(GLuint program, GLuint shader) = 0;

  virtual void shaderSource(
    GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) = 0;
  virtual void compileShader(GLuint shader) = 0;

  virtual void getShaderInfoLog(
    GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog) = 0;

  virtual void getShaderiv(GLuint shader, GLenum pname, GLint* params) = 0;

  virtual void uniform1f(GLint location, GLfloat v0) = 0;
  virtual void uniform2f(GLint location, GLfloat v0, GLfloat v1) = 0;
  virtual void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = 0;
  virtual void uniform4f(
    GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = 0;

  virtual void uniform1i(GLint location, GLint v0) = 0;
  virtual void uniform2i(GLint location, GLint v0, GLint v1) = 0;
  virtual void uniform3i(GLint location, GLint v0, GLint v1, GLint v2) = 0;
  virtual void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = 0;

  virtual void uniform1fv(GLint location, GLsizei count, const GLfloat* value) = 0;
  virtual void uniform2fv(GLint location, GLsizei count, const GLfloat* value) = 0;
  virtual void uniform3fv(GLint location, GLsizei count, const GLfloat* value) = 0;
  virtual void uniform4fv(GLint location, GLsizei count, const GLfloat* value) = 0;
  virtual void uniform1iv(GLint location, GLsizei count, const GLint* value) = 0;
  virtual void uniform2iv(GLint location, GLsizei count, const GLint* value) = 0;
  virtual void uniform3iv(GLint location, GLsizei count, const GLint* value) = 0;
  virtual void uniform4iv(GLint location, GLsizei count, const GLint* value) = 0;

  virtual void uniformMatrix2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix2x3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix3x2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix2x4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix4x2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix3x4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
  virtual void uniformMatrix4x3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

  virtual GLint getAttribLocation(GLuint program, const GLchar* name) = 0;
  virtual GLint getUniformLocation(GLuint program, const GLchar* name) = 0;

  virtual void genBuffers(GLsizei n, GLuint* buffers) = 0;
  virtual void deleteBuffers(GLsizei n, const GLuint* buffers) = 0;

  virtual void bindBuffer(GLenum target, GLuint buffer) = 0;
  virtual void bufferData(
    GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) = 0;
  virtual void bufferSubData(
    GLenum target, GLintptr offset, GLsizeiptr size, const void* data) = 0;

  virtual void vertexPointer(
    GLint size, GLenum type, GLsizei stride, const GLvoid* ptr) = 0;
  virtual void colorPointer(
    GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) = 0;
  virtual void normalPointer(GLenum type, GLsizei stride, const GLvoid* ptr) = 0;
  virtual void texCoordPointer(
    GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) = 0;

  virtual void enableVertexAttribArray(GLuint index) = 0;
  virtual void disableVertexAttribArray(GLuint index) = 0;

  virtual void vertexAttribPointer(
    GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const void* pointer) = 0;

  virtual void genTextures(GLsizei n, GLuint* textures) = 0;
  virtual void deleteTextures(GLsizei n, const GLuint* textures) = 0;

  virtual void bindTexture(GLenum target, GLuint texture) = 0;
  virtual void activeTexture(GLenum texture) = 0;

  virtual void texImage2D(
    GLenum target,
    GLint level,
    GLint internalFormat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* data) = 0;

  virtual void compressedTexImage2D(
    GLenum target,
    GLint level,
    GLenum internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLsizei imageSize,
    const GLvoid* data) = 0;

  virtual void texParameterf(GLenum target, GLenum pname, GLfloat param) = 0;
  virtual void texParameteri(GLenum target, GLenum pname, GLint param) = 0;

  virtual void pixelStoref(GLenum pname, GLfloat param) = 0;
  virtual void pixelStorei(GLenum pname, GLint param) = 0;

  virtual void clientActiveTexture(GLenum texture) = 0;

  virtual void drawArrays(GLenum mode, GLint first, GLsizei count) = 0;
  virtual void drawElements(
    GLenum mode, GLsizei count, GLenum type, const void* indices) = 0;
  virtual void multiDrawArrays(
    GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) = 0;

  virtual const GLubyte* getString(GLenum name) = 0;
  virtual GLenum getError() = 0;
};

} // namespace tb::gl
