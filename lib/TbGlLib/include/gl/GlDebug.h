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

#pragma once

#include <GL/glew.h>

#include "gl/GlInterface.h"

namespace tb::gl
{

class GlDebug : public Gl
{
private:
  Gl& m_gl;

public:
  explicit GlDebug(Gl& gl);

  void clear(GLbitfield mask) override;
  void clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) override;

  void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override;

  void matrixMode(GLenum mode) override;
  void loadMatrixd(const GLdouble* matrix) override;
  void loadMatrixf(const GLfloat* matrix) override;

  void getBooleanv(GLenum pname, GLboolean* params) override;
  void getDoublev(GLenum pname, GLdouble* params) override;
  void getFloatv(GLenum pname, GLfloat* params) override;
  void getIntegerv(GLenum pname, GLint* params) override;

  void enableClientState(GLenum cap) override;
  void disableClientState(GLenum cap) override;

  void pushAttrib(GLbitfield mask) override;
  void popAttrib() override;

  void enable(GLenum cap) override;
  void disable(GLenum cap) override;

  void lineWidth(GLfloat width) override;

  void polygonMode(GLenum face, GLenum mode) override;

  void frontFace(GLenum mode) override;
  void cullFace(GLenum mode) override;

  void blendFunc(GLenum sfactor, GLenum dfactor) override;

  void shadeModel(GLenum mode) override;

  void depthMask(GLboolean flag) override;
  void depthRange(GLclampd nearVal, GLclampd farVal) override;
  void depthFunc(GLenum func) override;

  GLuint createProgram() override;
  void deleteProgram(GLuint program) override;

  void linkProgram(GLuint program) override;

  void getProgramInfoLog(
    GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog) override;

  void getProgramiv(GLuint program, GLenum pname, GLint* params) override;

  void useProgram(GLuint program) override;

  GLuint createShader(GLenum shaderType) override;
  void deleteShader(GLuint shader) override;

  void attachShader(GLuint program, GLuint shader) override;

  void shaderSource(
    GLuint shader,
    GLsizei count,
    const GLchar* const* string,
    const GLint* length) override;
  void compileShader(GLuint shader) override;

  void getShaderInfoLog(
    GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog) override;

  void getShaderiv(GLuint shader, GLenum pname, GLint* params) override;

  void uniform1f(GLint location, GLfloat v0) override;
  void uniform2f(GLint location, GLfloat v0, GLfloat v1) override;
  void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override;
  void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override;

  void uniform1i(GLint location, GLint v0) override;
  void uniform2i(GLint location, GLint v0, GLint v1) override;
  void uniform3i(GLint location, GLint v0, GLint v1, GLint v2) override;
  void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override;

  void uniform1fv(GLint location, GLsizei count, const GLfloat* value) override;
  void uniform2fv(GLint location, GLsizei count, const GLfloat* value) override;
  void uniform3fv(GLint location, GLsizei count, const GLfloat* value) override;
  void uniform4fv(GLint location, GLsizei count, const GLfloat* value) override;
  void uniform1iv(GLint location, GLsizei count, const GLint* value) override;
  void uniform2iv(GLint location, GLsizei count, const GLint* value) override;
  void uniform3iv(GLint location, GLsizei count, const GLint* value) override;
  void uniform4iv(GLint location, GLsizei count, const GLint* value) override;

  void uniformMatrix2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix2x3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix3x2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix2x4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix4x2fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix3x4fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
  void uniformMatrix4x3fv(
    GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;

  GLint getAttribLocation(GLuint program, const GLchar* name) override;
  GLint getUniformLocation(GLuint program, const GLchar* name) override;

  void genBuffers(GLsizei n, GLuint* buffers) override;
  void deleteBuffers(GLsizei n, const GLuint* buffers) override;

  void bindBuffer(GLenum target, GLuint buffer) override;
  void bufferData(
    GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) override;
  void bufferSubData(
    GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override;

  void vertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* ptr) override;
  void colorPointer(
    GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) override;
  void normalPointer(GLenum type, GLsizei stride, const GLvoid* ptr) override;
  void texCoordPointer(
    GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) override;

  void enableVertexAttribArray(GLuint index) override;
  void disableVertexAttribArray(GLuint index) override;

  void vertexAttribPointer(
    GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const void* pointer) override;

  void genTextures(GLsizei n, GLuint* textures) override;
  void deleteTextures(GLsizei n, const GLuint* textures) override;

  void bindTexture(GLenum target, GLuint texture) override;
  void activeTexture(GLenum texture) override;

  void texImage2D(
    GLenum target,
    GLint level,
    GLint internalFormat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* data) override;

  void compressedTexImage2D(
    GLenum target,
    GLint level,
    GLenum internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLsizei imageSize,
    const GLvoid* data) override;

  void texParameterf(GLenum target, GLenum pname, GLfloat param) override;
  void texParameteri(GLenum target, GLenum pname, GLint param) override;

  void pixelStoref(GLenum pname, GLfloat param) override;
  void pixelStorei(GLenum pname, GLint param) override;

  void clientActiveTexture(GLenum texture) override;

  void drawArrays(GLenum mode, GLint first, GLsizei count) override;
  void drawElements(
    GLenum mode, GLsizei count, GLenum type, const void* indices) override;
  void multiDrawArrays(
    GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) override;

  const GLubyte* getString(GLenum name) override;
  GLenum getError() override;
};

} // namespace tb::gl
