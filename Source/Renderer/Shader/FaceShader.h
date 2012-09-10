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

#ifndef TrenchBroom_FaceShader_h
#define TrenchBroom_FaceShader_h

namespace TrenchBroom {
    namespace Renderer {
        namespace Shaders {
            static const String FaceVertexShader = "#version 120\n\
            void main(void) {\n\
                gl_Position = ftransform();\n\
                gl_TexCoord[0] = gl_MultiTexCoord0;\n\
            }\n";
            
            static const String FaceFragmentShader = "#version 120\n\
            uniform sampler2D FaceTexture;\n\
            \n\
            void main() {\n\
                gl_FragColor = texture2D(FaceTexture, vec2(0.5, 0.6));//gl_TexCoord[0].st);\n\
                // gl_FragColor = vec4(gl_TexCoord[0].st, 1.0, 1.0);\n\
            }\n ";
        }
    }
}

#endif
