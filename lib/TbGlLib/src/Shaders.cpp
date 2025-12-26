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

#include "gl/Shaders.h"

namespace tb::gl::Shaders
{

extern const ShaderConfig Grid2DShader = ShaderConfig{
  "2D Grid",
  {"Grid2D.vertsh"},
  {"Grid.fragsh", "Grid2D.fragsh"},
};

extern const ShaderConfig VaryingPCShader = ShaderConfig{
  "Varying Position / Color",
  {"VaryingPC.vertsh"},
  {"VaryingPC.fragsh"},
};

extern const ShaderConfig VaryingPUniformCShader = ShaderConfig{
  "Varying Position / Uniform Color",
  {"VaryingPUniformC.vertsh"},
  {"VaryingPC.fragsh"},
};

extern const ShaderConfig MiniMapEdgeShader = ShaderConfig{
  "MiniMap Edges",
  {"MiniMapEdge.vertsh"},
  {"MiniMapEdge.fragsh"},
};

extern const ShaderConfig EntityModelShader = ShaderConfig{
  "Entity Model",
  {"EntityModel.vertsh"},
  {"MapBounds.fragsh", "EntityModel.fragsh"},
};

extern const ShaderConfig FaceShader = ShaderConfig{
  "Face",
  {"Face.vertsh"},
  {"Grid.fragsh", "MapBounds.fragsh", "Face.fragsh"},
};

extern const ShaderConfig PatchShader = ShaderConfig{
  "Patch",
  {"Face.vertsh"},
  {"Grid.fragsh", "MapBounds.fragsh", "Face.fragsh"},
};

extern const ShaderConfig EdgeShader = ShaderConfig{
  "Edge",
  {"Edge.vertsh"},
  {"MapBounds.fragsh", "Edge.fragsh"},
};

extern const ShaderConfig ColoredTextShader = ShaderConfig{
  "Colored Text",
  {"ColoredText.vertsh"},
  {"Text.fragsh"},
};

extern const ShaderConfig TextShader = ShaderConfig{
  "Text",
  {"Text.vertsh"},
  {"Text.fragsh"},
};

extern const ShaderConfig TextBackgroundShader = ShaderConfig{
  "Text Background",
  {"TextBackground.vertsh"},
  {"TextBackground.fragsh"},
};

extern const ShaderConfig MaterialBrowserShader = ShaderConfig{
  "Material Browser",
  {"MaterialBrowser.vertsh"},
  {"MaterialBrowser.fragsh"},
};

extern const ShaderConfig MaterialBrowserBorderShader = ShaderConfig{
  "Material Browser Border",
  {"MaterialBrowserBorder.vertsh"},
  {"MaterialBrowserBorder.fragsh"},
};

extern const ShaderConfig HandleShader = ShaderConfig{
  "Handle",
  {"Handle.vertsh"},
  {"Handle.fragsh"},
};

extern const ShaderConfig ColoredHandleShader = ShaderConfig{
  "Colored Handle",
  {"ColoredHandle.vertsh"},
  {"Handle.fragsh"},
};

extern const ShaderConfig CompassShader = ShaderConfig{
  "Compass",
  {"Compass.vertsh"},
  {"Compass.fragsh"},
};

extern const ShaderConfig CompassOutlineShader = ShaderConfig{
  "Compass Outline",
  {"CompassOutline.vertsh"},
  {"Compass.fragsh"},
};

extern const ShaderConfig CompassBackgroundShader = ShaderConfig{
  "Compass Background",
  {"VaryingPUniformC.vertsh"},
  {"VaryingPC.fragsh"},
};

extern const ShaderConfig LinkLineShader = ShaderConfig{
  "Link Line",
  {"LinkLine.vertsh"},
  {"LinkLine.fragsh"},
};

extern const ShaderConfig LinkArrowShader = ShaderConfig{
  "Link Arrow",
  {"LinkArrow.vertsh"},
  {"LinkArrow.fragsh"},
};

extern const ShaderConfig TriangleShader = ShaderConfig{
  "Shaded Triangles",
  {"Triangle.vertsh"},
  {"Triangle.fragsh"},
};

extern const ShaderConfig UVViewShader = ShaderConfig{
  "UV View",
  {"UVView.vertsh"},
  {"UVView.fragsh"},
};

} // namespace tb::gl::Shaders
