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

#include "IO/FileManager.h"
#include "Model/Texture.h"
#include "Utility/Console.h"

#include <cassert>
#include <fstream>

namespace TrenchBroom {
    namespace Renderer {
        StringList Shader::loadSource(const String& path) {
            std::fstream stream(path.c_str());
            assert(stream.is_open());

            String line;
            StringList lines;
            
            while (!stream.eof()) {
                std::getline(stream, line);
                lines.push_back(line + '\n');
            }
            
            return lines;
        }
        
        Shader::Shader(const String& path, GLenum type, Utility::Console& console) :
        m_type(type),
        m_shaderId(0),
        m_console(console) {
            assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
            m_shaderId = glCreateShader(m_type);
            
            if (m_shaderId != 0) {
                IO::FileManager fileManager;
                m_name = fileManager.pathComponents(path).back();
                StringList source = loadSource(path);
                
                const char** linePtrs = new const char*[source.size()];
                for (unsigned int i = 0; i < source.size(); i++)
                    linePtrs[i] = source[i].c_str();
                
                glShaderSource(m_shaderId, static_cast<GLsizei>(source.size()), linePtrs, NULL);
                delete[] linePtrs;
                
                glCompileShader(m_shaderId);
                GLint compileStatus;
                glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &compileStatus);
                
                if (compileStatus != 0)
                    m_console.debug("Created %s", m_name.c_str());
                else
                    m_console.error("Unable to compile %s, compilation output was:", m_name.c_str());

				GLint infoLogLength;
                glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
				if (infoLogLength > 0) {
					char* infoLog = new char[infoLogLength];
					glGetShaderInfoLog(m_shaderId, infoLogLength, &infoLogLength, infoLog);
                
					if (compileStatus != 0)
						m_console.debug(infoLog);
					else
	                    m_console.error(infoLog);
					delete [] infoLog;
				}
            } else {
                m_console.error("Unable to create %s", m_name.c_str());
            }
        }
        
        Shader::~Shader() {
            if (m_shaderId != 0) {
                glDeleteShader(m_shaderId);
                m_shaderId = 0;
            }
        }

        void Shader::attachTo(GLuint programId) {
            glAttachShader(programId, m_shaderId);
        }

        void Shader::detachFrom(GLuint programId) {
            glDetachShader(programId, m_shaderId);
        }
    }
}
