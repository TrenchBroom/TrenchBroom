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

#ifndef TrenchBroom_RendererTypes_h
#define TrenchBroom_RendererTypes_h

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class StringManager;
        }
        
        class EntityRendererManager;
        class Shader;
        class ShaderProgram;
        class VertexArray;
        class Vbo;
        
        typedef std::auto_ptr<EntityRendererManager> EntityRendererManagerPtr;
        typedef std::auto_ptr<Shader> ShaderPtr;
        typedef std::auto_ptr<ShaderProgram> ShaderProgramPtr;
        typedef std::auto_ptr<Text::StringManager> StringManagerPtr;
        typedef std::auto_ptr<VertexArray> VertexArrayPtr;
        typedef std::auto_ptr<Vbo> VboPtr;
    }
}

#endif
