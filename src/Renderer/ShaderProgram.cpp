/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
            m_programId = glCreateProgram();
            if (m_programId == 0)
                throw RenderException("Cannot create shader program " + m_name);
        }
        
        ShaderProgram::~ShaderProgram() {
            if (m_programId != 0) {
                glDeleteProgram(m_programId);
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
            
            glUseProgram(m_programId);
            assert(checkActive());
        }

        void ShaderProgram::deactivate() {
            glUseProgram(0);
        }

        void ShaderProgram::set(const String& name, const bool value) {
            return set(name, static_cast<int>(value));
        }

        void ShaderProgram::set(const String& name, const int value) {
            assert(checkActive());
            glUniform1i(findUniformLocation(name), value);
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const float value) {
            assert(checkActive());
            glUniform1f(findUniformLocation(name), value);
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Vec2f& value) {
            assert(checkActive());
            glUniform2f(findUniformLocation(name), value.x(), value.y());
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Vec3f& value) {
            assert(checkActive());
            glUniform3f(findUniformLocation(name), value.x(), value.y(), value.z());
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Vec4f& value) {
            assert(checkActive());
            glUniform4f(findUniformLocation(name), value.x(), value.y(), value.z(), value.w());
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Mat2x2f& value) {
            assert(checkActive());
            glUniformMatrix2fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v));
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Mat3x3f& value) {
            assert(checkActive());
            glUniformMatrix3fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v));
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::set(const String& name, const Mat4x4f& value) {
            assert(checkActive());
            glUniformMatrix4fv(findUniformLocation(name), 1, false, reinterpret_cast<const float*>(value.v));
            assert(glGetError() == GL_NO_ERROR);
        }

        void ShaderProgram::link() {
            glLinkProgram(m_programId);
            
            GLint linkStatus = 0;
            glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
            
            if (linkStatus == 0) {
                RenderException ex;
                ex << "Cannot link shader program " << m_name << ": ";

                GLint infoLogLength = 0;
                glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
                if (infoLogLength > 0) {
                    char* infoLog = new char[infoLogLength];
                    glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog);
                    infoLog[infoLogLength-1] = 0;

                    ex << infoLog;
                    delete [] infoLog;
                } else {
                    ex << "Unknown error";
                }
                
                throw ex;
            }
            assert(glGetError() == GL_NO_ERROR);

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
            glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);
            return static_cast<GLuint>(currentProgramId) == m_programId;
        }
    }
}
