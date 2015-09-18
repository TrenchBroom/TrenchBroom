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

#ifndef TrenchBroom_Shaders
#define TrenchBroom_Shaders

#include "Renderer/ShaderConfig.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Shaders {
            extern const ShaderConfig Grid2DShader;
            extern const ShaderConfig VaryingPCShader;
            extern const ShaderConfig VaryingPUniformCShader;
            extern const ShaderConfig MiniMapEdgeShader;
            extern const ShaderConfig EntityModelShader;
            extern const ShaderConfig FaceShader;
            extern const ShaderConfig ColoredTextShader;
            extern const ShaderConfig TextBackgroundShader;
            extern const ShaderConfig TextureBrowserShader;
            extern const ShaderConfig TextureBrowserBorderShader;
            extern const ShaderConfig BrowserGroupShader;
            extern const ShaderConfig HandleShader;
            extern const ShaderConfig ColoredHandleShader;
            extern const ShaderConfig CompassShader;
            extern const ShaderConfig CompassOutlineShader;
            extern const ShaderConfig CompassBackgroundShader;
            extern const ShaderConfig EntityLinkShader;
            extern const ShaderConfig TriangleShader;
            extern const ShaderConfig UVViewShader;
        }
    }
}

#endif /* defined(TrenchBroom_Shaders) */
