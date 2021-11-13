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

#include "Ensure.h"
#include "Renderer/GL.h"
#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboManager.h"

#include <kdl/vector_utils.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace Renderer {
enum class PrimType;

/**
 * Represents an array of vertices. Optionally, multiple instances of this class can share the same
 * data. Vertex arrays can be copied around without incurring the cost of copying the actual data.
 *
 * A vertex array can be uploaded into a vertex buffer object by calling the prepare method.
 * Furthermore, a vertex array can be rendered by calling one of the provided render methods.
 */
class VertexArray {
private:
  class BaseHolder {
  public:
    virtual ~BaseHolder();

    virtual size_t vertexCount() const = 0;
    virtual size_t sizeInBytes() const = 0;

    virtual void prepare(VboManager& vboManager) = 0;
    virtual void setup() = 0;
    virtual void cleanup() = 0;
  };

  template <typename VertexSpec> class Holder : public BaseHolder {
  private:
    VboManager* m_vboManager;
    Vbo* m_vbo;
    size_t m_vertexCount;

  public:
    size_t vertexCount() const override { return m_vertexCount; }

    size_t sizeInBytes() const override { return VertexSpec::Size * m_vertexCount; }

    void prepare(VboManager& vboManager) override {
      if (m_vertexCount > 0 && m_vbo == nullptr) {
        m_vboManager = &vboManager;
        m_vbo = vboManager.allocateVbo(VboType::ArrayBuffer, sizeInBytes());
        ;
        m_vbo->writeBuffer(0, doGetVertices());
      }
    }

    void setup() override {
      ensure(m_vbo != nullptr, "block is null");
      m_vbo->bind();
      VertexSpec::setup(m_vboManager->shaderManager().currentProgram(), m_vbo->offset());
    }

    void cleanup() override {
      VertexSpec::cleanup(m_vboManager->shaderManager().currentProgram());
      m_vbo->unbind();
    }

  protected:
    Holder(const size_t vertexCount)
      : m_vboManager(nullptr)
      , m_vbo(nullptr)
      , m_vertexCount(vertexCount) {}

    ~Holder() override {
      // TODO: Revisit this revisiting OpenGL resource management. We should not store the
      // VboManager, since it represents a safe time to delete the OpenGL buffer object.
      if (m_vbo != nullptr) {
        m_vboManager->destroyVbo(m_vbo);
        m_vbo = nullptr;
      }
    }

  private:
    using VertexList = std::vector<typename VertexSpec::Vertex>;
    virtual const VertexList& doGetVertices() const = 0;
  };

  template <typename VertexSpec> class ByValueHolder : public Holder<VertexSpec> {
  private:
    using VertexList = std::vector<typename VertexSpec::Vertex>;

  private:
    VertexList m_vertices;

  public:
    ByValueHolder(const VertexList& vertices)
      : Holder<VertexSpec>(vertices.size())
      , m_vertices(vertices) {}

    ByValueHolder(VertexList&& vertices)
      : Holder<VertexSpec>(vertices.size())
      , m_vertices(std::move(vertices)) {}

    void prepare(VboManager& vboManager) override {
      Holder<VertexSpec>::prepare(vboManager);
      kdl::vec_clear_to_zero(m_vertices);
    }

  private:
    const VertexList& doGetVertices() const override { return m_vertices; }
  };

  template <typename VertexSpec> class ByRefHolder : public Holder<VertexSpec> {
  private:
    using VertexList = std::vector<typename VertexSpec::Vertex>;

  private:
    const VertexList& m_vertices;

  public:
    ByRefHolder(const VertexList& vertices)
      : Holder<VertexSpec>(vertices.size())
      , m_vertices(vertices) {}

  private:
    const VertexList& doGetVertices() const override { return m_vertices; }
  };

private:
  std::shared_ptr<BaseHolder> m_holder;
  bool m_prepared;
  bool m_setup;

public:
  /**
   * Creates a new empty vertex array.
   */
  VertexArray();

  /**
   * Creates a new vertex array by copying the given vertices. After this operation, the given
   * vector of vertices is left unchanged.
   *
   * @tparam Attrs the vertex attribute types
   * @param vertices the vertices to copy
   * @return the vertex array
   */
  template <typename... Attrs>
  static VertexArray copy(const std::vector<GLVertex<Attrs...>>& vertices) {
    return VertexArray(
      std::make_shared<ByValueHolder<typename GLVertex<Attrs...>::Type>>(vertices));
  }

  /**
   * Creates a new vertex array by moving the contents of the given vertices.
   *
   * @tparam Attrs the vertex attribute types
   * @param vertices the vertices to move
   * @return the vertex array
   */
  template <typename... Attrs> static VertexArray move(std::vector<GLVertex<Attrs...>>&& vertices) {
    return VertexArray(
      std::make_shared<ByValueHolder<typename GLVertex<Attrs...>::Type>>(std::move(vertices)));
  }

  /**
   * Creates a new vertex array by referencing the contents of the given vertices. After this
   * operation, the given vector of vertices is left unchanged. Since this vertex array will only
   * store a reference to the given vector, changes to the given vector are reflected in this array.
   *
   * A caller must ensure that this vertex array does not outlive the given vector of vertices.
   *
   * @tparam Attrs the vertex attribute types
   * @param vertices the vertices to reference
   * @return the vertex array
   */
  template <typename... Attrs>
  static VertexArray ref(const std::vector<GLVertex<Attrs...>>& vertices) {
    return VertexArray(std::make_shared<ByRefHolder<typename GLVertex<Attrs...>::Type>>(vertices));
  }

  /**
   * Indicates whether this vertex array is empty.
   *
   * @return true if this vertex array is empty and false otherwise
   */
  bool empty() const;

  /**
   * Returns the size of this vertex array in bytes.
   *
   * @return the size of this vertex array in bytes
   */
  size_t sizeInBytes() const;

  /**
   * Returns the numnber of vertices stored in this vertex array.
   *
   * @return the number of vertices stored in this vertex array
   */
  size_t vertexCount() const;

  /**
   * Indicates whether this vertex array way prepared. Preparing a vertex array uploads its data
   * into a vertex buffer object.
   *
   * @return true if this vertex array was prepared and false otherwise
   */
  bool prepared() const;

  /**
   * Prepares this vertex array by uploading its contents into the given vertex buffer object.
   *
   * @param vboManager the vertex buffer object to upload the contents of this vertex array into
   */
  void prepare(VboManager& vboManager);

  /**
   * Sets this vertex array up for rendering. If this vertex array is only rendered once, then there
   * is no need to call this method (or the corresponding cleanup method), since the render methods
   * will perform setup and cleanup automatically unless the vertex array is already set up when the
   * render method is called.
   *
   * In the case the the vertex array was already setup before a render method was called, the
   * render method will skip the setup and cleanup, and it is the callers' responsibility to perform
   * proper cleanup.
   *
   * It is only useful to perform setup and cleanup for a caller if the caller intends to issue
   * multiple render calls to this vertex array.
   */
  bool setup();

  /**
   * Renders this vertex array as a range of primitives of the given type.
   *
   * @param primType the primitive type to render
   */
  void render(PrimType primType);

  /**
   * Renders a sub range of this vertex array as a range of primitives of the given type.
   *
   * @param primType the primitive type to render
   * @param index the index of the first vertex in this vertex array to render
   * @param count the number of vertices to render
   */
  void render(PrimType primType, GLint index, GLsizei count);

  /**
   * Renders a number of sub ranges of this vertex array as ranges of primitives of the given type.
   * The given indices array contains the start indices of the ranges to render, while the given
   * counts array contains the length of the range. Both the indices and counts must contain at
   * least primCount elements.
   *
   * @param primType the primitive type to render
   * @param indices the start indices of the ranges to render
   * @param counts the lengths of the ranges to render
   * @param primCount the number of ranges to render
   */
  void render(PrimType primType, const GLIndices& indices, const GLCounts& counts, GLint primCount);

  /**
   * Renders a number of primitives of the given type, the vertices of which are indicates by the
   * given index array.
   *
   * @param primType the primitive type to render
   * @param indices the indices of the vertices to render
   * @param count the number of vertices to render
   */
  void render(PrimType primType, const GLIndices& indices, GLsizei count);
  void cleanup();

private:
  explicit VertexArray(std::shared_ptr<BaseHolder> holder);
};
} // namespace Renderer
} // namespace TrenchBroom
