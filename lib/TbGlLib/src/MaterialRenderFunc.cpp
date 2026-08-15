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

#include "gl/MaterialRenderFunc.h"

#include "gl/ActiveShader.h"
#include "gl/Material.h"
#include "gl/Texture.h"

namespace tb::gl
{

MaterialRenderFunc::~MaterialRenderFunc() = default;
void MaterialRenderFunc::before(Gl&, const Material*) {}
void MaterialRenderFunc::after(Gl&, const Material*) {}

DefaultMaterialRenderFunc::DefaultMaterialRenderFunc(
  const int minFilter, const int magFilter)
  : m_minFilter{minFilter}
  , m_magFilter{magFilter}
{
}

void DefaultMaterialRenderFunc::before(Gl& gl, const Material* material)
{
  if (material)
  {
    material->activate(gl, m_minFilter, m_magFilter);
  }
}

void DefaultMaterialRenderFunc::after(Gl& gl, const Material* material)
{
  if (material)
  {
    material->deactivate(gl);
  }
}

void setAlphaFuncUniforms(ActiveShader& shader, const Material* material)
{
  const auto alphaFunc =
    material ? material->effectiveAlphaFunc() : std::optional<MaterialAlphaFunc>{};
  shader.set("EnableMasked", alphaFunc.has_value());
  if (alphaFunc)
  {
    shader.set("AlphaFuncCompare", static_cast<size_t>(alphaFunc->compare));
    shader.set("AlphaFuncThreshold", alphaFunc->threshold);
  }
}

AlphaTestedMaterialRenderFunc::AlphaTestedMaterialRenderFunc(
  ActiveShader& shader, const int minFilter, const int magFilter)
  : DefaultMaterialRenderFunc{minFilter, magFilter}
  , m_shader{shader}
{
}

void AlphaTestedMaterialRenderFunc::before(Gl& gl, const Material* material)
{
  DefaultMaterialRenderFunc::before(gl, material);
  setAlphaFuncUniforms(m_shader, material);
}

} // namespace tb::gl
