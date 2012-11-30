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

#include "ShaderProgram.h"

#include "IO/FileManager.h"
#include "Model/Texture.h"
#include "Renderer/Shader/Shader.h"
#include "Utility/Console.h"

#include <cassert>
#include <fstream>

namespace TrenchBroom {
    namespace Renderer {
        GLint ShaderProgram::uniformLocation(const String& name) {
            UniformVariableMap::iterator it = m_uniformVariables.find(name);
            if (it == m_uniformVariables.end()) {
                GLint index = glGetUniformLocation(m_programId, name.c_str());
                if (index == -1)
                    m_console.warn("Location of uniform variable '%s' could not be found in %s", name.c_str(), m_name.c_str());
                m_uniformVariables[name] = index;
                return index;
            }
            
            return it->second;
        }
        
        bool ShaderProgram::checkActive() {
            GLint currentProgramId;
            glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);
            return static_cast<GLuint>(currentProgramId) == m_programId;
        }

        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console) :
        m_name(name),
        m_programId(0),
        m_needsLinking(true),
        m_console(console) {
            m_programId = glCreateProgram();
            
            if (m_programId != 0) {
                m_console.debug("Created %s", m_name.c_str());
            } else {
                m_console.error("Unable to create %s", m_name.c_str());
            }
        }
        
        ShaderProgram::~ShaderProgram() {
            if (m_programId != 0) {
                glDeleteProgram(m_programId);
                m_programId = 0;
            }
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
        
        bool ShaderProgram::activate() {
            if (m_programId == 0)
                return false;
            
            if (m_needsLinking) {
                m_uniformVariables.clear();
                
                glLinkProgram(m_programId);
                
                GLint linkStatus;
                glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
                
                if (linkStatus == 0)
                    m_console.error("Unable to link %s, linker output was:", m_name.c_str());
                
				GLint infoLogLength;
                glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
				if (infoLogLength > 0) {
					char* infoLog = new char[infoLogLength];
					glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog);
                    
	                if (linkStatus == 0)
		                m_console.error(infoLog);
			        else
				        m_console.debug(infoLog);
					delete [] infoLog;
				}
                
                // always set to false to prevent console spam
                m_needsLinking = false;
            }
            
            glUseProgram(m_programId);
            return true;
        }
        
        void ShaderProgram::deactivate() {
            glUseProgram(0);
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, bool value) {
            return setUniformVariable(name, static_cast<int>(value));
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, int value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform1i(location, value);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, float value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform1f(location, value);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec2f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform2f(location, value.x, value.y);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec3f& value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform3f(location, value.x, value.y, value.z);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec4f& value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform4f(location, value.x, value.y, value.z, value.w);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat2f& value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix2fv(location, 1, false, value.v);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat3f& value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix3fv(location, 1, false, value.v);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat4f& value) {
            assert(checkActive());
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix4fv(location, 1, false, value.v);
            return true;
        }
    }
}