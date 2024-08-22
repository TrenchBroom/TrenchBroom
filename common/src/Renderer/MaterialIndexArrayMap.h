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

#include "Renderer/IndexArrayMap.h"

#include <unordered_map>

namespace TrenchBroom
{
namespace Assets
{
class Material;
}

namespace Renderer
{
class IndexArray;
class MaterialRenderFunc;

/**
 * Manages ranges of primitives to be rendered using indices stored in an IndexArray
 * instance. To avoid costly material switching, the instances are grouped by their
 * materials.
 */
class MaterialIndexArrayMap
{
public:
  using Material = Assets::Material;

private:
  using MaterialToIndexArrayMap = std::unordered_map<const Material*, IndexArrayMap>;

public:
  /**
   * Helper class that allows to record sizing information to initialize a material index
   * array map to the desired size.
   */
  class Size
  {
  private:
    friend class MaterialIndexArrayMap;

    using MaterialToSize = std::unordered_map<const Material*, IndexArrayMap::Size>;
    MaterialToSize m_sizes;
    size_t m_indexCount;

  public:
    /**
     * Creates a new instance.
     */
    Size();

    /**
     * Increase the storage for the given primitive type by the given number of indices.
     *
     * @param material the material for the primitive
     * @param primType the primitive type
     * @param count the number of primitives to account for
     */
    void inc(const Material* material, PrimType primType, size_t count);

    /**
     * Increase the storage by the given size.
     *
     * @param material the material
     * @param size the size to increment by
     */
    void inc(const Material* material, const IndexArrayMap::Size& size);

    /**
     * The total number of indices that have been accounted for.
     *
     * @return the total number of indices
     */
    size_t indexCount() const;

  private:
    void initialize(MaterialToIndexArrayMap& ranges) const;
  };

private:
  MaterialToIndexArrayMap m_ranges;

public:
  /**
   * Creates a new empty index array map that allows for dynamic growth. Note that dynamic
   * growth can incur performance costs when data buffers have to be reallocated as they
   * grow.
   */
  MaterialIndexArrayMap();

  /**
   * Creates a new index array map and initializes the internal data structures using the
   * given size information.
   *
   * @param size the size to initialize to
   */
  explicit MaterialIndexArrayMap(const Size& size);

  /**
   * Returns the size of this  index array map. A  index array map initialized with the
   * returned size can hold exactly the same data as this index array map.
   *
   * @return the size of this index array map
   */
  Size size() const;

  /**
   * Adds the given number of primitives of the given type to this range map with the
   * given material. Effectively, the range of primitives of the given type that has been
   * recorded so far with the given material is extended by the given number of indices.
   *
   * @param material the material for the primitive
   * @param primType the type of primitive
   * @param count the number of indices
   * @return the offset of the next block that would be recorded for the given primitive
   * type
   */
  size_t add(const Material* material, PrimType primType, size_t count);

  /**
   * Renders the recorded primitives using the indices stored in the given index array.
   * The primitives are batched by their associated materials. The given render function
   * type provides two callbacks. One is called before all primitives with a given
   * material is rendered, and one is called afterwards.
   *
   * @param indexArray the index array to render
   * @param func the material callbacks
   */
  void render(IndexArray& indexArray, MaterialRenderFunc& func);
};
} // namespace Renderer
} // namespace TrenchBroom
