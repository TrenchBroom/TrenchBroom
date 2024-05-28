/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Renderer/MaterialIndexRangeMap.h"
#include "Renderer/VertexArray.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Assets
{
class Material;
}

namespace Renderer
{
class VboManager;
class MaterialRenderFunc;

class MaterialRenderer
{
public:
  virtual ~MaterialRenderer();

  virtual bool empty() const = 0;

  virtual void prepare(VboManager& vboManager) = 0;
  virtual void render() = 0;
  virtual void render(MaterialRenderFunc& func) = 0;
};

class MaterialIndexRangeRenderer : public MaterialRenderer
{
private:
  VertexArray m_vertexArray;
  MaterialIndexRangeMap m_indexRange;

public:
  MaterialIndexRangeRenderer();
  MaterialIndexRangeRenderer(
    const VertexArray& vertexArray, const MaterialIndexRangeMap& indexRange);
  MaterialIndexRangeRenderer(
    const VertexArray& vertexArray,
    const Assets::Material* material,
    const IndexRangeMap& indexRange);
  ~MaterialIndexRangeRenderer() override;

  bool empty() const override;

  void prepare(VboManager& vboManager) override;
  void render() override;
  void render(MaterialRenderFunc& func) override;
};

class MultiMaterialIndexRangeRenderer : public MaterialRenderer
{
private:
  std::vector<std::unique_ptr<MaterialIndexRangeRenderer>> m_renderers;

public:
  MultiMaterialIndexRangeRenderer(
    std::vector<std::unique_ptr<MaterialIndexRangeRenderer>> renderers);
  ~MultiMaterialIndexRangeRenderer() override;

  bool empty() const override;

  void prepare(VboManager& vboManager) override;
  void render() override;
  void render(MaterialRenderFunc& func) override;
};
} // namespace Renderer
} // namespace TrenchBroom
