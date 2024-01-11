/*
 Copyright (C) 2020 Kristian Duske

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

#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom::Renderer
{
class RenderContext;
class RenderBatch;
class VboManager;

class LinkRenderer : public DirectRenderable
{
public:
  using LineVertex = GLVertexTypes::P3C4::Vertex;

  struct ArrowPositionName
  {
    static inline const auto name = std::string{"arrowPosition"};
  };

  struct LineDirName
  {
    static inline const auto name = std::string{"lineDir"};
  };

  using ArrowVertex = GLVertexType<
    GLVertexAttributeTypes::P3, // vertex of the arrow (exposed in shader as gl_Vertex)
    GLVertexAttributeTypes::C4, // arrow color (exposed in shader as gl_Color)
    GLVertexAttributeUser<ArrowPositionName, GL_FLOAT, 3, false>,    // arrow position
    GLVertexAttributeUser<LineDirName, GL_FLOAT, 3, false>>::Vertex; // direction the
                                                                     // arrow is pointing
private:
  VertexArray m_lines;
  VertexArray m_arrows;

  bool m_valid = false;

public:
  LinkRenderer();

  void render(RenderContext& renderContext, RenderBatch& renderBatch);
  void invalidate();

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;

  void renderLines(RenderContext& renderContext);
  void renderArrows(RenderContext& renderContext);

  void validate();

  virtual std::vector<LinkRenderer::LineVertex> getLinks() = 0;

  deleteCopy(LinkRenderer);
};

} // namespace TrenchBroom::Renderer
