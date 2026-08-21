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

namespace tb::gl
{
class ActiveShader;
class Gl;
class Material;

class MaterialRenderFunc
{
public:
  virtual ~MaterialRenderFunc();
  virtual void before(Gl& gl, const Material* material);
  virtual void after(Gl& gl, const Material* material);
};

class DefaultMaterialRenderFunc : public MaterialRenderFunc
{
private:
  int m_minFilter;
  int m_magFilter;

public:
  DefaultMaterialRenderFunc(int minFilter, int magFilter);

  void before(Gl& gl, const Material* material) override;
  void after(Gl& gl, const Material* material) override;
};

/**
 * Sets the EnableMasked/AlphaFuncCompare/AlphaFuncThreshold uniforms on shader from
 * material's effectiveAlphaFunc(), so the shader's alpha-test cutout matches the
 * material's derived or overridden alpha-test state. material may be null.
 */
void setAlphaFuncUniforms(ActiveShader& shader, const Material* material);

/**
 * Like DefaultMaterialRenderFunc, but also sets the alpha-test uniforms via
 * setAlphaFuncUniforms(), for shaders that support alpha-test cutout.
 */
class AlphaTestedMaterialRenderFunc : public DefaultMaterialRenderFunc
{
private:
  ActiveShader& m_shader;

public:
  AlphaTestedMaterialRenderFunc(ActiveShader& shader, int minFilter, int magFilter);

  void before(Gl& gl, const Material* material) override;
};

} // namespace tb::gl
