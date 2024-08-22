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
#include "Renderer/PrimType.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboManager.h"

#include "kdl/vector_utils.h"

#include <memory>

namespace TrenchBroom
{
namespace Renderer
{
/**
 * Represents an array of indices. Optionally, multiple instances of this class can share
 * the same data. Index arrays can be copied around without incurring the cost of copying
 * the actual data.
 *
 * An index array can be uploaded into a vertex buffer object by calling the prepare
 * method. Furthermore, an index array can be rendered by calling the provided render
 * method.
 */
class IndexArray
{
private:
  class BaseHolder
  {
  public:
    using Ptr = std::shared_ptr<BaseHolder>;
    virtual ~BaseHolder() {}

    virtual size_t indexCount() const = 0;
    virtual size_t sizeInBytes() const = 0;

    virtual void prepare(VboManager& vboManager) = 0;
    virtual void setup() = 0;
    virtual void cleanup() = 0;

  public:
    void render(PrimType primType, size_t offset, size_t count) const;

  private:
    virtual void doRender(PrimType primType, size_t offset, size_t count) const = 0;
  };

  template <typename Index>
  class Holder : public BaseHolder
  {
  protected:
    using IndexList = std::vector<Index>;

  private:
    VboManager* m_vboManager;
    Vbo* m_vbo;
    size_t m_indexCount;

  public:
    size_t indexCount() const override { return m_indexCount; }

    size_t sizeInBytes() const override { return sizeof(Index) * m_indexCount; }

    virtual void prepare(VboManager& vboManager) override
    {
      if (m_indexCount > 0 && m_vbo == nullptr)
      {
        m_vboManager = &vboManager;
        m_vbo = vboManager.allocateVbo(VboType::ElementArrayBuffer, sizeInBytes());
        m_vbo->writeBuffer(0, doGetIndices());
      }
    }

    void setup() override
    {
      ensure(m_vbo != nullptr, "block is null");
      m_vbo->bind();
    }

    void cleanup() override { m_vbo->unbind(); }

  protected:
    Holder(const size_t indexCount)
      : m_vboManager{nullptr}
      , m_vbo{nullptr}
      , m_indexCount{indexCount}
    {
    }

    virtual ~Holder() override
    {
      // TODO: Revisit this revisiting OpenGL resource management. We should not store the
      // VboManager, since it represents a safe time to delete the OpenGL buffer object.
      if (m_vbo != nullptr)
      {
        m_vboManager->destroyVbo(m_vbo);
        m_vbo = nullptr;
      }
    }

  private:
    void doRender(PrimType primType, size_t offset, size_t count) const override
    {
      glAssert(glDrawElements(
        toGL(primType),
        static_cast<GLsizei>(count),
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>(offset * 4u)));
    }

  private:
    virtual const IndexList& doGetIndices() const = 0;
  };

  template <typename Index>
  class ByValueHolder : public Holder<Index>
  {
  public:
    using IndexList = typename Holder<Index>::IndexList;

  private:
    IndexList m_indices;

  public:
    ByValueHolder(IndexList indices)
      : Holder<Index>{indices.size()}
      , m_indices{std::move(indices)}
    {
    }

    void prepare(VboManager& vboManager)
    {
      Holder<Index>::prepare(vboManager);
      kdl::vec_clear_to_zero(m_indices);
    }

  private:
    const IndexList& doGetIndices() const { return m_indices; }
  };

  template <typename Index>
  class ByRefHolder : public Holder<Index>
  {
  public:
    using IndexList = typename Holder<Index>::IndexList;

  private:
    const IndexList& m_indices;

  public:
    ByRefHolder(const IndexList& indices)
      : Holder<Index>{indices.size()}
      , m_indices{indices}
    {
    }

  private:
    const IndexList& doGetIndices() const { return m_indices; }
  };

private:
  BaseHolder::Ptr m_holder;
  bool m_prepared;
  bool m_setup;

public:
  /**
   * Creates a new empty index array.
   */
  IndexArray();

  /**
   * Creates a new index array by copying the given indices. After this operation, the
   * given vector of indices is left unchanged.
   *
   * @tparam Index the index type
   * @param indices the indices to copy
   * @return the index array
   */
  template <typename Index>
  static IndexArray copy(const std::vector<Index>& indices)
  {
    return IndexArray(BaseHolder::Ptr(new ByValueHolder<Index>(indices)));
  }

  /**
   * Creates a new index array by swapping the contents of the given indices. After this
   * operation, the given vector of indices is empty.
   *
   * @tparam Index the index type
   * @param indices the indices to swap
   * @return the index array
   */
  template <typename Index>
  static IndexArray move(std::vector<Index>&& indices)
  {
    return IndexArray(BaseHolder::Ptr(new ByValueHolder<Index>(std::move(indices))));
  }

  /**
   * Creates a new index array by referencing the contents of the given indices. After
   * this operation, the given vector of indices is left unchanged. Since this index array
   * will only store a reference to the given vector, changes to the given vector are
   * reflected in this array.
   *
   * A caller must ensure that this index array does not outlive the given vector of
   * indices.
   *
   * @tparam Index the index type
   * @param indices the indices to copy
   * @return the index array
   */
  template <typename Index>
  static IndexArray ref(const std::vector<Index>& indices)
  {
    return IndexArray(BaseHolder::Ptr(new ByRefHolder<Index>(indices)));
  }

  /**
   * Indicates whether this index array is empty.
   *
   * @return true if this index array is empty and false otherwise
   */
  bool empty() const;

  /**
   * Returns the size of this index array in bytes.
   *
   * @return the size of this index array in bytes
   */
  size_t sizeInBytes() const;

  /**
   * Returns the number of indices in this index array.
   *
   * @return the number of indices in this index array
   */
  size_t indexCount() const;

  /**
   * Indicates whether this index array was prepared.
   *
   * @return true if this index array was prepared
   */
  bool prepared() const;

  /**
   * Prepares this index array by uploading its contents into the given vertex buffer
   * object.
   *
   * @param vboManager the vertex buffer object to upload the contents of this index array
   * into
   */
  void prepare(VboManager& vboManager);

  /**
   * Sets this index array up for rendering. If this index array is only rendered once,
   * then there is no need to call this method (or the corresponding cleanup method),
   * since the render method will perform setup and cleanup automatically unless the index
   * array is already set up when the render method is called.
   *
   * In the case the the index array was already setup before a render method was called,
   * the render method will skip the setup and cleanup, and it is the callers'
   * responsibility to perform proper cleanup.
   *
   * It is only useful to perform setup and cleanup for a caller if the caller intends to
   * issue multiple render calls to this index array.
   */
  bool setup();

  /**
   * Renders a range of primitives of the given type using the indices stored in this
   * index array. Assumes that an appropriate vertex array has been set up that contains
   * the actual vertex data.
   *
   * @param primType the type of primitive to render
   * @param offset the offset of the range of indices to render
   * @param count the number of indices to render
   */
  void render(PrimType primType, size_t offset, size_t count);

  void cleanup();

private:
  explicit IndexArray(BaseHolder::Ptr holder);
};
} // namespace Renderer
} // namespace TrenchBroom
