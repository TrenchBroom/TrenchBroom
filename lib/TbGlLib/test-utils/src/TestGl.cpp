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

#include "gl/TestGl.h"

namespace tb::gl
{

void TestGl::clear(GLbitfield) {}
void TestGl::clearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}

void TestGl::viewport(GLint, GLint, GLsizei, GLsizei) {}

void TestGl::matrixMode(GLenum) {}
void TestGl::loadMatrixd(const GLdouble*) {}
void TestGl::loadMatrixf(const GLfloat*) {}

void TestGl::getBooleanv(GLenum, GLboolean*) {}
void TestGl::getDoublev(GLenum, GLdouble*) {}
void TestGl::getFloatv(GLenum, GLfloat*) {}
void TestGl::getIntegerv(GLenum, GLint*) {}

void TestGl::enableClientState(GLenum) {}
void TestGl::disableClientState(GLenum) {}

void TestGl::pushAttrib(GLbitfield) {}
void TestGl::popAttrib() {}

void TestGl::enable(GLenum) {}
void TestGl::disable(GLenum) {}

void TestGl::lineWidth(GLfloat) {}

void TestGl::polygonMode(GLenum, GLenum) {}

void TestGl::frontFace(GLenum) {}
void TestGl::cullFace(GLenum) {}

void TestGl::blendFunc(GLenum, GLenum) {}

void TestGl::shadeModel(GLenum) {}

void TestGl::depthMask(GLboolean) {}
void TestGl::depthRange(GLclampd, GLclampd) {}
void TestGl::depthFunc(GLenum) {}

GLuint TestGl::createProgram()
{
  return 0u;
}

void TestGl::deleteProgram(GLuint) {}

void TestGl::linkProgram(GLuint) {}

void TestGl::getProgramInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}

void TestGl::getProgramiv(GLuint, GLenum, GLint*) {}

void TestGl::useProgram(GLuint) {}

GLuint TestGl::createShader(GLenum)
{
  return 0u;
}

void TestGl::deleteShader(GLuint) {}

void TestGl::attachShader(GLuint, GLuint) {}

void TestGl::shaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*) {}
void TestGl::compileShader(GLuint) {}

void TestGl::getShaderInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}

void TestGl::getShaderiv(GLuint, GLenum, GLint*) {}

void TestGl::uniform1f(GLint, GLfloat) {}
void TestGl::uniform2f(GLint, GLfloat, GLfloat) {}
void TestGl::uniform3f(GLint, GLfloat, GLfloat, GLfloat) {}
void TestGl::uniform4f(GLint, GLfloat, GLfloat, GLfloat, GLfloat) {}

void TestGl::uniform1i(GLint, GLint) {}
void TestGl::uniform2i(GLint, GLint, GLint) {}
void TestGl::uniform3i(GLint, GLint, GLint, GLint) {}
void TestGl::uniform4i(GLint, GLint, GLint, GLint, GLint) {}

void TestGl::uniform1fv(GLint, GLsizei, const GLfloat*) {}
void TestGl::uniform2fv(GLint, GLsizei, const GLfloat*) {}
void TestGl::uniform3fv(GLint, GLsizei, const GLfloat*) {}
void TestGl::uniform4fv(GLint, GLsizei, const GLfloat*) {}
void TestGl::uniform1iv(GLint, GLsizei, const GLint*) {}
void TestGl::uniform2iv(GLint, GLsizei, const GLint*) {}
void TestGl::uniform3iv(GLint, GLsizei, const GLint*) {}
void TestGl::uniform4iv(GLint, GLsizei, const GLint*) {}

void TestGl::uniformMatrix2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix2x3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix3x2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix2x4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix4x2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix3x4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
void TestGl::uniformMatrix4x3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}

GLint TestGl::getAttribLocation(GLuint, const GLchar*)
{
  return -1;
}

GLint TestGl::getUniformLocation(const GLuint, const GLchar*)
{
  return -1;
}

void TestGl::genBuffers(GLsizei, GLuint*) {}
void TestGl::deleteBuffers(GLsizei, const GLuint*) {}

void TestGl::bindBuffer(GLenum, GLuint) {}
void TestGl::bufferData(GLenum, GLsizeiptr, const GLvoid*, GLenum) {}
void TestGl::bufferSubData(GLenum, GLintptr, GLsizeiptr, const void*) {}

void TestGl::vertexPointer(GLint, GLenum, GLsizei, const GLvoid*) {}
void TestGl::colorPointer(GLint, GLenum, GLsizei, const GLvoid*) {}
void TestGl::normalPointer(GLenum, GLsizei, const GLvoid*) {}
void TestGl::texCoordPointer(GLint, GLenum, GLsizei, const GLvoid*) {}

void TestGl::enableVertexAttribArray(GLuint) {}
void TestGl::disableVertexAttribArray(GLuint) {}

void TestGl::vertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)
{
}

void TestGl::genTextures(GLsizei, GLuint*) {}
void TestGl::deleteTextures(GLsizei, const GLuint*) {}

void TestGl::bindTexture(GLenum, GLuint) {}
void TestGl::activeTexture(GLenum) {}

void TestGl::texImage2D(
  GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)
{
}

void TestGl::compressedTexImage2D(
  GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*)
{
}

void TestGl::texParameterf(GLenum, GLenum, GLfloat) {}
void TestGl::texParameteri(GLenum, GLenum, GLint) {}

void TestGl::pixelStoref(GLenum, GLfloat) {}
void TestGl::pixelStorei(GLenum, GLint) {}

void TestGl::clientActiveTexture(GLenum) {}

void TestGl::drawArrays(GLenum, GLint, GLsizei) {}
void TestGl::drawElements(GLenum, GLsizei, GLenum, const void*) {}
void TestGl::multiDrawArrays(GLenum, const GLint*, const GLsizei*, GLsizei) {}

const GLubyte* TestGl::getString(GLenum)
{
  return nullptr;
}

GLenum TestGl::getError()
{
  return GL_NO_ERROR;
}

} // namespace tb::gl
