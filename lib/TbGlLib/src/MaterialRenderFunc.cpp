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

#include "gl/GlUtils.h"
#include "gl/Material.h"
#include "gl/Texture.h"

namespace tb::gl
{

MaterialRenderFunc::~MaterialRenderFunc() = default;
void MaterialRenderFunc::before(const Material*) {}
void MaterialRenderFunc::after(const Material*) {}

DefaultMaterialRenderFunc::DefaultMaterialRenderFunc(
  const int minFilter, const int magFilter)
  : m_minFilter{minFilter}
  , m_magFilter{magFilter}
{
}

void DefaultMaterialRenderFunc::before(const Material* material)
{
  if (material)
  {
    material->activate(m_minFilter, m_magFilter);
  }
}

void DefaultMaterialRenderFunc::after(const Material* material)
{
  if (material)
  {
    material->deactivate();
  }
}

} // namespace tb::gl
