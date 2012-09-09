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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Shader.h"

#include "Utility/Console.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Shader::Shader(GLenum type, const String& source, Utility::Console& console) :
        m_type(type),
        m_source(source),
        m_console(console),
        m_shaderId(0) {
            assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
        }
        
        Shader::~Shader() {
            if (m_shaderId != 0) {
                glDeleteShader(m_shaderId);
                m_shaderId = 0;
            }
        }

        bool Shader::createShader() {
            assert(m_shaderId == 0);
            m_shaderId = glCreateShader(m_type);
            if (m_shaderId == 0) {
                m_console.error("Unable to create OpenGL shader");
                return false;
            }
            
            const char* cSource = m_source.c_str();
            glShaderSource(m_shaderId, 1, &cSource, NULL);
            glCompileShader(m_shaderId);
            
            GLint compileStatus;
            glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &compileStatus);

            GLint infoLogLength;
            glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
            char infoLog[infoLogLength];
            glGetShaderInfoLog(m_shaderId, infoLogLength, &infoLogLength, infoLog);
            
            if (compileStatus == 0) {
                m_console.error(infoLog);
                return false;
            } else {
                m_console.debug(infoLog);
            }
            
            return true;
        }

        void Shader::attachTo(GLuint programId) {
            glAttachShader(programId, m_shaderId);
        }

        void Shader::detachFrom(GLuint programId) {
            glDetachShader(programId, m_shaderId);
        }
        
        ShaderProgram::ShaderProgram(Utility::Console& console) :
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {}
        
        ShaderProgram::~ShaderProgram() {
            if (m_programId != 0) {
                glDeleteProgram(m_programId);
                m_programId = NULL;
            }
        }
        
        bool ShaderProgram::createProgram() {
            assert(m_programId == 0);
            m_programId = glCreateProgram();
            
            if (m_programId == 0) {
                m_console.error("Unable to create OpenGL program");
                return false;
            }
            return true;
        }

        void ShaderProgram::attachShader(Shader& shader) {
            assert(m_programId != 0);
            shader.attachTo(m_programId);
            m_needsLinking = true;
        }
        
        void ShaderProgram::detachShader(Shader& shader) {
            assert(m_programId != 0);
            shader.detachFrom(m_programId);
            m_needsLinking = true;
        }
        
        void ShaderProgram::activate() {
            assert(m_programId != 0);
            
            if (m_needsLinking) {
                glLinkProgram(m_programId);

                GLint linkStatus;
                glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
                
                GLint infoLogLength;
                glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
                char infoLog[infoLogLength];
                glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog);
                
                if (linkStatus == 0) {
                    m_console.error(infoLog);
                } else {
                    m_console.debug(infoLog);
                    m_needsLinking = false;
                }
            }
            
            if (!m_needsLinking) {
                glUseProgram(m_programId);
            }
        }
        
        void ShaderProgram::deactivate() {
            glUseProgram(0);
        }
    }
}