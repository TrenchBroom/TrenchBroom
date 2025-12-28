/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include "gl/ShaderConfig.h"

namespace tb::gl::Shaders
{

inline const ShaderConfig Grid2DShader = ShaderConfig{
  "2D Grid",
  {"Grid2D.vertsh"},
  {"Grid.fragsh", "Grid2D.fragsh"},
};

inline const ShaderConfig VaryingPCShader = ShaderConfig{
  "Varying Position / Color",
  {"VaryingPC.vertsh"},
  {"VaryingPC.fragsh"},
};

inline const ShaderConfig VaryingPUniformCShader = ShaderConfig{
  "Varying Position / Uniform Color",
  {"VaryingPUniformC.vertsh"},
  {"VaryingPC.fragsh"},
};

inline const ShaderConfig MiniMapEdgeShader = ShaderConfig{
  "MiniMap Edges",
  {"MiniMapEdge.vertsh"},
  {"MiniMapEdge.fragsh"},
};

inline const ShaderConfig EntityModelShader = ShaderConfig{
  "Entity Model",
  {"EntityModel.vertsh"},
  {"MapBounds.fragsh", "EntityModel.fragsh"},
};

inline const ShaderConfig FaceShader = ShaderConfig{
  "Face",
  {"Face.vertsh"},
  {"Grid.fragsh", "MapBounds.fragsh", "Face.fragsh"},
};

inline const ShaderConfig PatchShader = ShaderConfig{
  "Patch",
  {"Face.vertsh"},
  {"Grid.fragsh", "MapBounds.fragsh", "Face.fragsh"},
};

inline const ShaderConfig EdgeShader = ShaderConfig{
  "Edge",
  {"Edge.vertsh"},
  {"MapBounds.fragsh", "Edge.fragsh"},
};

inline const ShaderConfig ColoredTextShader = ShaderConfig{
  "Colored Text",
  {"ColoredText.vertsh"},
  {"Text.fragsh"},
};

inline const ShaderConfig TextShader = ShaderConfig{
  "Text",
  {"Text.vertsh"},
  {"Text.fragsh"},
};

inline const ShaderConfig TextBackgroundShader = ShaderConfig{
  "Text Background",
  {"TextBackground.vertsh"},
  {"TextBackground.fragsh"},
};

inline const ShaderConfig MaterialBrowserShader = ShaderConfig{
  "Material Browser",
  {"MaterialBrowser.vertsh"},
  {"MaterialBrowser.fragsh"},
};

inline const ShaderConfig MaterialBrowserBorderShader = ShaderConfig{
  "Material Browser Border",
  {"MaterialBrowserBorder.vertsh"},
  {"MaterialBrowserBorder.fragsh"},
};

inline const ShaderConfig HandleShader = ShaderConfig{
  "Handle",
  {"Handle.vertsh"},
  {"Handle.fragsh"},
};

inline const ShaderConfig ColoredHandleShader = ShaderConfig{
  "Colored Handle",
  {"ColoredHandle.vertsh"},
  {"Handle.fragsh"},
};

inline const ShaderConfig CompassShader = ShaderConfig{
  "Compass",
  {"Compass.vertsh"},
  {"Compass.fragsh"},
};

inline const ShaderConfig CompassOutlineShader = ShaderConfig{
  "Compass Outline",
  {"CompassOutline.vertsh"},
  {"Compass.fragsh"},
};

inline const ShaderConfig CompassBackgroundShader = ShaderConfig{
  "Compass Background",
  {"VaryingPUniformC.vertsh"},
  {"VaryingPC.fragsh"},
};

inline const ShaderConfig LinkLineShader = ShaderConfig{
  "Link Line",
  {"LinkLine.vertsh"},
  {"LinkLine.fragsh"},
};

inline const ShaderConfig LinkArrowShader = ShaderConfig{
  "Link Arrow",
  {"LinkArrow.vertsh"},
  {"LinkArrow.fragsh"},
};

inline const ShaderConfig TriangleShader = ShaderConfig{
  "Shaded Triangles",
  {"Triangle.vertsh"},
  {"Triangle.fragsh"},
};

inline const ShaderConfig UVViewShader = ShaderConfig{
  "UV View",
  {"UVView.vertsh"},
  {"UVView.fragsh"},
};

} // namespace tb::gl::Shaders
