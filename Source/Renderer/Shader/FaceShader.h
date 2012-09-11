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
            varying vec4 modelCoordinates;\n\
            varying vec3 modelNormal;\n\
            \n\
            void main(void) {\n\
                gl_Position = ftransform();\n\
                gl_TexCoord[0] = gl_MultiTexCoord0;\n\
                modelCoordinates = gl_Vertex;\n\
                modelNormal = gl_Normal;\n\
            }\n";
            
            static const String FaceFragmentShader = "#version 120\n\
            uniform float Brightness;\n\
            uniform sampler2D FaceTexture;\n\
            uniform bool ApplyTinting;\n\
            uniform vec4 TintColor;\n\
            uniform bool GrayScale;\n\
            uniform bool RenderGrid;\n\
            uniform float GridSize;\n\
            uniform vec4 GridColor;\n\
            \n\
            varying vec4 modelCoordinates;\n\
            varying vec3 modelNormal;\n\
            \n\
            void gridXY() {\n\
                if (floor(mod(modelCoordinates.x + 0.5, GridSize)) == 0.0 || \n\
                    floor(mod(modelCoordinates.y + 0.5, GridSize)) == 0.0)\n\
                    gl_FragColor = vec4(mix(gl_FragColor.rgb, GridColor.rgb, GridColor.a), gl_FragColor.a);\n\
            }\n\
            \n\
            void gridXZ() {\n\
                if (floor(mod(modelCoordinates.x + 0.5, GridSize)) == 0.0 || \n\
                    floor(mod(modelCoordinates.z + 0.5, GridSize)) == 0.0)\n\
                    gl_FragColor = vec4(mix(gl_FragColor.rgb, GridColor.rgb, GridColor.a), gl_FragColor.a);\n\
            }\n\
            \n\
            void gridYZ() {\n\
                if (floor(mod(modelCoordinates.y + 0.5, GridSize)) == 0.0 || \n\
                    floor(mod(modelCoordinates.z + 0.5, GridSize)) == 0.0)\n\
                    gl_FragColor = vec4(mix(gl_FragColor.rgb, GridColor.rgb, GridColor.a), gl_FragColor.a);\n\
            }\n\
            \n\
            void main() {\n\
                vec4 texel = texture2D(FaceTexture, gl_TexCoord[0].st);\n\
                gl_FragColor = vec4(vec3(Brightness * texel), texel.a);\n\
                gl_FragColor = clamp(2 * gl_FragColor, 0.0, 1.0);\n\
                \n\
                if (GrayScale) {\n\
                    float gray = dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114));\n\
                    gl_FragColor = vec4(gray, gray, gray, gl_FragColor.a);\n\
                }\n\
                \n\
                if (ApplyTinting) {\n\
                    gl_FragColor = vec4(gl_FragColor.rgb * TintColor.rgb * TintColor.a, gl_FragColor.a);\n\
                    gl_FragColor = clamp(2.0 * gl_FragColor, 0.0, 1.0);\n\
                }\n\
                \n\
                if (RenderGrid) {\n\
                    float normX = abs(modelNormal.x);\n\
                    float normY = abs(modelNormal.y);\n\
                    float normZ = abs(modelNormal.z);\n\
                    if (normX > normY) {\n\
                        if (normX > normZ) \n\
                            gridYZ();\n\
                        else\n\
                            gridXY();\n\
                    } else if (normY > normZ) {\n\
                        gridXZ();\n\
                    } else {\n\
                        gridXY();\n\
                    }\n\
                }\n\
            }\n ";
        }
    }
}

#endif
