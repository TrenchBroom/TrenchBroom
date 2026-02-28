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

#include "gl/GlInterface.h"

namespace tb::gl
{

class TestGl : public Gl
{
public:
  void clear(GLbitfield) override;
  void clearColor(GLfloat, GLfloat, GLfloat, GLfloat) override;

  void viewport(GLint, GLint, GLsizei, GLsizei) override;

  void matrixMode(GLenum) override;
  void loadMatrixd(const GLdouble*) override;
  void loadMatrixf(const GLfloat*) override;

  void getBooleanv(GLenum, GLboolean*) override;
  void getDoublev(GLenum, GLdouble*) override;
  void getFloatv(GLenum, GLfloat*) override;
  void getIntegerv(GLenum, GLint*) override;

  void enableClientState(GLenum) override;
  void disableClientState(GLenum) override;

  void pushAttrib(GLbitfield) override;
  void popAttrib() override;

  void enable(GLenum) override;
  void disable(GLenum) override;

  void lineWidth(GLfloat) override;

  void polygonMode(GLenum, GLenum) override;

  void frontFace(GLenum) override;
  void cullFace(GLenum) override;

  void blendFunc(GLenum, GLenum) override;

  void shadeModel(GLenum) override;

  void depthMask(GLboolean) override;
  void depthRange(GLclampd, GLclampd) override;
  void depthFunc(GLenum) override;

  GLuint createProgram() override;
  void deleteProgram(GLuint) override;

  void linkProgram(GLuint) override;

  void getProgramInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) override;

  void getProgramiv(GLuint, GLenum, GLint*) override;

  void useProgram(GLuint) override;

  GLuint createShader(GLenum) override;
  void deleteShader(GLuint) override;

  void attachShader(GLuint, GLuint) override;

  void shaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*) override;
  void compileShader(GLuint) override;

  void getShaderInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) override;

  void getShaderiv(GLuint, GLenum, GLint*) override;

  void uniform1f(GLint, GLfloat) override;
  void uniform2f(GLint, GLfloat, GLfloat) override;
  void uniform3f(GLint, GLfloat, GLfloat, GLfloat) override;
  void uniform4f(GLint, GLfloat, GLfloat, GLfloat, GLfloat) override;

  void uniform1i(GLint, GLint) override;
  void uniform2i(GLint, GLint, GLint) override;
  void uniform3i(GLint, GLint, GLint, GLint) override;
  void uniform4i(GLint, GLint, GLint, GLint, GLint) override;

  void uniform1fv(GLint, GLsizei, const GLfloat*) override;
  void uniform2fv(GLint, GLsizei, const GLfloat*) override;
  void uniform3fv(GLint, GLsizei, const GLfloat*) override;
  void uniform4fv(GLint, GLsizei, const GLfloat*) override;
  void uniform1iv(GLint, GLsizei, const GLint*) override;
  void uniform2iv(GLint, GLsizei, const GLint*) override;
  void uniform3iv(GLint, GLsizei, const GLint*) override;
  void uniform4iv(GLint, GLsizei, const GLint*) override;

  void uniformMatrix2fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix3fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix2x3fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix3x2fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix2x4fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix4x2fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix3x4fv(GLint, GLsizei, GLboolean, const GLfloat*) override;
  void uniformMatrix4x3fv(GLint, GLsizei, GLboolean, const GLfloat*) override;

  GLint getAttribLocation(GLuint, const GLchar*) override;
  GLint getUniformLocation(GLuint, const GLchar*) override;

  void genBuffers(GLsizei, GLuint*) override;
  void deleteBuffers(GLsizei, const GLuint*) override;

  void bindBuffer(GLenum, GLuint) override;
  void bufferData(GLenum, GLsizeiptr, const GLvoid*, GLenum) override;
  void bufferSubData(GLenum, GLintptr, GLsizeiptr, const void*) override;

  void vertexPointer(GLint, GLenum, GLsizei, const GLvoid*) override;
  void colorPointer(GLint, GLenum, GLsizei, const GLvoid*) override;
  void normalPointer(GLenum, GLsizei, const GLvoid*) override;
  void texCoordPointer(GLint, GLenum, GLsizei, const GLvoid*) override;

  void enableVertexAttribArray(GLuint) override;
  void disableVertexAttribArray(GLuint) override;

  void vertexAttribPointer(
    GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) override;

  void genTextures(GLsizei, GLuint*) override;
  void deleteTextures(GLsizei, const GLuint*) override;

  void bindTexture(GLenum, GLuint) override;
  void activeTexture(GLenum) override;

  void texImage2D(
    GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)
    override;

  void compressedTexImage2D(
    GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) override;

  void texParameterf(GLenum, GLenum, GLfloat) override;
  void texParameteri(GLenum, GLenum, GLint) override;

  void pixelStoref(GLenum, GLfloat) override;
  void pixelStorei(GLenum, GLint) override;

  void clientActiveTexture(GLenum) override;

  void drawArrays(GLenum, GLint, GLsizei) override;
  void drawElements(GLenum, GLsizei, GLenum, const void*) override;
  void multiDrawArrays(GLenum, const GLint*, const GLsizei*, GLsizei) override;

  const GLubyte* getString(GLenum name) override;
  GLenum getError() override;
};

} // namespace tb::gl