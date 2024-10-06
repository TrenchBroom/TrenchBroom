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

#include "Renderer/IndexArray.h"
#include "Renderer/MaterialIndexArrayMap.h"
#include "Renderer/VertexArray.h"

namespace tb::Assets
{
class Material;
}

namespace tb::Renderer
{
class VboManager;
class MaterialRenderFunc;

class MaterialIndexArrayRenderer
{
private:
  VertexArray m_vertexArray;
  IndexArray m_indexArray;
  MaterialIndexArrayMap m_indexRanges;

public:
  MaterialIndexArrayRenderer();
  MaterialIndexArrayRenderer(
    VertexArray vertexArray, IndexArray indexArray, MaterialIndexArrayMap indexArrayMap);

  bool empty() const;

  void prepare(VboManager& vboManager);
  void render(MaterialRenderFunc& func);
};

} // namespace tb::Renderer
