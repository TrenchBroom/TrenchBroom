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

#include "Color.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

#include <memory>

namespace TrenchBroom {
namespace Renderer {
class RenderBatch;

class EdgeRenderer {
public:
  struct Params {
    float width;
    double offset;
    bool onTop;
    bool useColor;
    Color color;
    Params(float i_width, double i_offset, bool i_onTop);
    Params(float i_width, double i_offset, bool i_onTop, const Color& i_color);
    Params(float i_width, double i_offset, bool i_onTop, bool i_useColor, const Color& i_color);
  };

  class RenderBase {
  private:
    const Params m_params;

  public:
    RenderBase(const Params& params);
    virtual ~RenderBase();

  protected:
    void renderEdges(RenderContext& renderContext);

  private:
    virtual void doRenderVertices(RenderContext& renderContext) = 0;
  };

public:
  virtual ~EdgeRenderer();

  void render(RenderBatch& renderBatch, float width = 1.0f, double offset = 0.0);
  void render(
    RenderBatch& renderBatch, const Color& color, float width = 1.0f, double offset = 0.0);
  void render(
    RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f,
    double offset = 0.0);
  void renderOnTop(RenderBatch& renderBatch, float width = 1.0f, double offset = 0.2);
  void renderOnTop(
    RenderBatch& renderBatch, const Color& color, float width = 1.0f, double offset = 0.2);
  void renderOnTop(
    RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f,
    double offset = 0.2);
  void render(
    RenderBatch& renderBatch, bool useColor, const Color& color, bool onTop, float width,
    double offset);

private:
  virtual void doRender(RenderBatch& renderBatch, const Params& params) = 0;
};

class DirectEdgeRenderer : public EdgeRenderer {
private:
  class Render : public RenderBase, public DirectRenderable {
  private:
    VertexArray m_vertexArray;
    IndexRangeMap m_indexRanges;

  public:
    Render(const Params& params, VertexArray& vertexArray, IndexRangeMap& indexRanges);

  private:
    void doPrepareVertices(VboManager& vboManager) override;
    void doRender(RenderContext& renderContext) override;
    void doRenderVertices(RenderContext& renderContext) override;
  };

private:
  VertexArray m_vertexArray;
  IndexRangeMap m_indexRanges;

public:
  DirectEdgeRenderer();
  DirectEdgeRenderer(const VertexArray& vertexArray, const IndexRangeMap& indexRanges);
  DirectEdgeRenderer(const VertexArray& vertexArray, PrimType primType);

  DirectEdgeRenderer(const DirectEdgeRenderer& other);
  DirectEdgeRenderer& operator=(DirectEdgeRenderer other);

  friend void swap(DirectEdgeRenderer& left, DirectEdgeRenderer& right);

private:
  void doRender(RenderBatch& renderBatch, const EdgeRenderer::Params& params) override;
};

// Only used by BrushRenderer

class BrushEdgeRenderer {
public:
  struct Params {
    float width;
    double offset;
    bool onTop;
    bool useColor;
    Color color;
    Params(float i_width, double i_offset, bool i_onTop);
    Params(float i_width, double i_offset, bool i_onTop, const Color& i_color);
    Params(float i_width, double i_offset, bool i_onTop, bool i_useColor, const Color& i_color);
  };

  class RenderBase {
  private:
    const Params m_params;

  public:
    RenderBase(const Params& params);
    virtual ~RenderBase();

  protected:
    void renderEdges(RenderContext& renderContext);

  private:
    virtual void doRenderVertices(RenderContext& renderContext) = 0;
  };

public:
  virtual ~BrushEdgeRenderer();

  void render(RenderBatch& renderBatch, float width = 1.0f, double offset = 0.0);
  void render(
    RenderBatch& renderBatch, const Color& color, float width = 1.0f, double offset = 0.0);
  void render(
    RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f,
    double offset = 0.0);
  void renderOnTop(RenderBatch& renderBatch, float width = 1.0f, double offset = 0.2);
  void renderOnTop(
    RenderBatch& renderBatch, const Color& color, float width = 1.0f, double offset = 0.2);
  void renderOnTop(
    RenderBatch& renderBatch, bool useColor, const Color& color, float width = 1.0f,
    double offset = 0.2);
  void render(
    RenderBatch& renderBatch, bool useColor, const Color& color, bool onTop, float width,
    double offset);

private:
  virtual void doRender(RenderBatch& renderBatch, const Params& params) = 0;
};

class DirectBrushEdgeRenderer : public BrushEdgeRenderer {
private:
  class Render : public RenderBase, public DirectRenderable {
  private:
    std::shared_ptr<BrushEdgeVertexArray> m_vertexArray;

  public:
    Render(const Params& params, std::shared_ptr<BrushEdgeVertexArray> vertexArray);

  private:
    void doPrepareVertices(VboManager& vboManager) override;
    void doRender(RenderContext& renderContext) override;
    void doRenderVertices(RenderContext& renderContext) override;
  };

private:
  std::shared_ptr<BrushEdgeVertexArray> m_vertexArray;

public:
  DirectBrushEdgeRenderer();
  DirectBrushEdgeRenderer(std::shared_ptr<BrushEdgeVertexArray> vertexArray);

  DirectBrushEdgeRenderer(const DirectBrushEdgeRenderer& other);
  DirectBrushEdgeRenderer& operator=(DirectBrushEdgeRenderer other);

  friend void swap(DirectBrushEdgeRenderer& left, DirectBrushEdgeRenderer& right);

private:
  void doRender(RenderBatch& renderBatch, const BrushEdgeRenderer::Params& params) override;
};

// FIXME: IndexedBrushEdgeRenderer
class IndexedEdgeRenderer : public BrushEdgeRenderer {
private:
  class Render : public RenderBase, public IndexedRenderable {
  private:
    std::shared_ptr<BrushVertexArray> m_vertexArray;
    std::shared_ptr<BrushIndexArray> m_indexArray;

  public:
    Render(
      const Params& params, std::shared_ptr<BrushVertexArray> vertexArray,
      std::shared_ptr<BrushIndexArray> indexArray);

  private:
    void prepareVerticesAndIndices(VboManager& vboManager) override;
    void doRender(RenderContext& renderContext) override;
    void doRenderVertices(RenderContext& renderContext) override;
  };

private:
  std::shared_ptr<BrushVertexArray> m_vertexArray;
  std::shared_ptr<BrushIndexArray> m_indexArray;

public:
  IndexedEdgeRenderer();
  IndexedEdgeRenderer(
    std::shared_ptr<BrushVertexArray> vertexArray, std::shared_ptr<BrushIndexArray> indexArray);

  IndexedEdgeRenderer(const IndexedEdgeRenderer& other);
  IndexedEdgeRenderer& operator=(IndexedEdgeRenderer other);

  friend void swap(IndexedEdgeRenderer& left, IndexedEdgeRenderer& right);

private:
  void doRender(RenderBatch& renderBatch, const BrushEdgeRenderer::Params& params) override;
};
} // namespace Renderer
} // namespace TrenchBroom
