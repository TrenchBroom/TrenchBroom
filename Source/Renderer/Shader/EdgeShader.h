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

#ifndef TrenchBroom_EdgeVertexShader_h
#define TrenchBroom_EdgeVertexShader_h

namespace TrenchBroom {
    namespace Renderer {
        namespace Shaders {
            static const String ColoredEdgeVertexShader = "#version 120\n\
            varying vec4 vertexColor;\n\
            \n\
            void main(void) {\n\
                vertexColor = gl_Color;\n\
                gl_Position = ftransform();\n\
            }\n";
            
            static const String ConstantColoredEdgeVertexShader = "#version 120\n\
            uniform vec4 Color;\n\
            \n\
            varying vec4 vertexColor;\n\
            \n\
            void main(void) {\n\
                vertexColor = Color;\n\
                gl_Position = ftransform();\n\
            }\n";
            
            static const String EdgeFragmentShader = "#version 120\n\
            varying vec4 vertexColor;\n\
            void main() {\n\
                gl_FragColor = vertexColor;\n\
            }\n ";
        }
    }
}

#endif
