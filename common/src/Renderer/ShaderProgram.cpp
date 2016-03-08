/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "ShaderProgram.h"

#include "Exceptions.h"
#include "Renderer/Shader.h"

namespace TrenchBroom {
    namespace Renderer {
        ShaderProgram::ShaderProgram(const String& name) :
        m_name(name),
        m_programId(0),
        m_needsLinking(true) {
            glAssert(m_programId = glCreateProgram());
            if (m_programId == 0)
                throw RenderException("Cannot create shader program " + m_name);
        }
        
        ShaderProgram::~ShaderProgram() {
            if (m_programId != 0) {
                glAssert(glDeleteProgram(m_programId));
                m_programId = 0;
            }
        }

        void ShaderProgram::attach(Shader& shader) {
            assert(m_programId != 0);
            shader.attach(m_programId);
            m_needsLinking = true;
        }
        
        void ShaderProgram::detach(Shader& shader) {
            assert(m_programId != 0);
            shader.detach(m_programId);
            m_needsLinking = true;
        }

        void ShaderProgram::activate() {
            assert(m_programId != 0);
            
            if (m_needsLinking)
                link();
            
            glAssert(glUseProgram(m_programId));
            assert(checkActive());
        }

        void ShaderProgram::deactivate() {
            glAssert(glUseProgram(0));
        }

        void ShaderProgram::set(const String& name, const bool value) {
            return set(name, static_cast<int>(value));
        }

        void ShaderProgram::set(const String& name, const int value) {
            assert(checkActive());
            glAssert(glUniform1i(findUniformLocation(name), value));
        }

        void ShaderProgram::set(const String& name, const size_t value) {
            assert(checkActive());
            glAssert(glUniform1i(findUniformLocation(name), static_cast<int>(value)));
        }

        void ShaderProgram::set(const String& name, const float value) {
            assert(checkActive());
            glAssert(glUniform1f(findUniformLocation(name), value));
        }

        void ShaderProgram::set(const String& name, const double value) {
            /* FIXME using glUniform1d gives undefined references on Linux and Windows builds
            assert(checkActive());
            glUniform1d(findUniformLocation(name), value);
            assert(glGetError() == GL_NO_ERROR);
             */

            set(name, static_cast<float>(value));
        }
        
        void ShaderProgram::set(const String& name, const Vec2f& value) {
            assert(checkActive());
            glAssert(glUniform2f(findUniformLocation(name), value.x(), value.y()));
        }

        void ShaderProgram::set(const String& name, const Vec3f& value) {
            assert(checkActive());
            glAssert(glUniform3f(findUniformLocation(name), value.x(), value.y(), value.z()));
        }

        void ShaderProgram::set(const String& name, const Vec4f& value) {
            assert(checkActive());
            glAssert(glUniform4f(findUniformLocation(name), value.x(), value.y(), value.z(), value.w()));
        }

        void ShaderProgram::set(const String& name, const Mat2x2f& value) {
            assert(checkActive());
            glAssert(glUniformMatrix2fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v)));
        }

        void ShaderProgram::set(const String& name, const Mat3x3f& value) {
            assert(checkActive());
            glAssert(glUniformMatrix3fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v)));
        }

        void ShaderProgram::set(const String& name, const Mat4x4f& value) {
            assert(checkActive());
            glAssert(glUniformMatrix4fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v)));
        }

        void ShaderProgram::link() {
            glAssert(glLinkProgram(m_programId));
            
            GLint linkStatus = 0;
            glAssert(glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus));
            
            if (linkStatus == 0) {
                RenderException ex;
                ex << "Cannot link shader program " << m_name << ": ";

                GLint infoLogLength = 0;
                glAssert(glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength));
                if (infoLogLength > 0) {
                    char* infoLog = new char[infoLogLength];
                    glAssert(glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog));
                    infoLog[infoLogLength-1] = 0;

                    ex << infoLog;
                    delete [] infoLog;
                } else {
                    ex << "Unknown error";
                }
                
                throw ex;
            }

            m_variableCache.clear();
            m_needsLinking = false;
        }

        GLint ShaderProgram::findUniformLocation(const String& name) const {
            UniformVariableCache::iterator it = m_variableCache.find(name);
            if (it == m_variableCache.end()) {
                const GLint index = glGetUniformLocation(m_programId, name.c_str());
                if (index == -1)
                    throw RenderException("Location of uniform variable '" + name + "' could not be found in shader program " + m_name);
                
                m_variableCache[name] = index;
                return index;
            }
            return it->second;
        }

        bool ShaderProgram::checkActive() const {
            GLint currentProgramId = -1;
            glAssert(glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId));
            return static_cast<GLuint>(currentProgramId) == m_programId;
        }
    }
}
