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

#include <cstddef>
#include <vector>

namespace TrenchBroom {
namespace Renderer {
template <typename... AttrTypes> struct GLVertexType;

/**
 * Stores the attribute values of an OpenGL vertex such as the position, the texture coordinates or
 * the normal.
 *
 * Refer to the corresponding vertex type to see how to use vertices in conjunction with OpenGL
 * vertex buffers.
 *
 * @tparam AttrTypes the attribute types
 */
template <typename... AttrTypes> struct GLVertex;

/**
 * Template specialization of the GLVertex template for the case of multiple vertex attributes. The
 * first attribute is handled in this template, while the remaining attributes are delegated to
 * another GLVertex that is stored inside of this struct in the "rest" attribute.
 *
 * Together, this struct and the "rest" attribute form a recursive structure that ends when all but
 * one vertex attributes have been "peeled off" the given attribute type parameter pack.
 *
 * The design of this struct achieves standard layout so that a vertex can be uploaded directly into
 * an OpenGL vertex buffer.
 *
 * @tparam AttrType the type of the vertex attribute stored here
 * @tparam AttrTypeRest the types of the remaining vertex attributes
 */
template <typename AttrType, typename... AttrTypeRest> struct GLVertex<AttrType, AttrTypeRest...> {
  using Type = GLVertexType<AttrType, AttrTypeRest...>;

  // To achieve standard memory layout and to allow a vector of vertices to be uploaded, this
  // template stores the value of its first attribute type parameter as a member, and the remaining
  // attribute values using the 'rest' attribute.
  typename AttrType::ElementType attr;
  GLVertex<AttrTypeRest...> rest;

  /**
   * Creates a new vertex.
   */
  GLVertex() = default;

  // Copy and move constructors
  GLVertex(const GLVertex<AttrType, AttrTypeRest...>& other) = default;
  GLVertex(GLVertex<AttrType, AttrTypeRest...>&& other) noexcept = default;

  // Assignment operators
  GLVertex<AttrType, AttrTypeRest...>& operator=(const GLVertex<AttrType, AttrTypeRest...>& other) =
    default;
  GLVertex<AttrType, AttrTypeRest...>& operator=(
    GLVertex<AttrType, AttrTypeRest...>&& other) noexcept = default;

  // explicitly declare the following two constructors instead of using type deduction with an
  // rvalue reference to avoid any clashes with the copy / move constructors

  /**
   * Creates a new vertex by moving the given attribute values into this vertex.
   *
   * @param i_attr the value of the first attribute
   * @param i_rest the values of the remaining attributes
   */
  explicit GLVertex(
    typename AttrType::ElementType&& i_attr, typename AttrTypeRest::ElementType&&... i_rest)
    : attr(std::move(i_attr))
    , rest(std::move(i_rest)...) {}

  /**
   * Creates a new vertex by copying the given attribute values into this vertex.
   *
   * @param i_attr the value of the first attribute
   * @param i_rest the value of the remaining attributes
   */
  explicit GLVertex(
    const typename AttrType::ElementType& i_attr,
    const typename AttrTypeRest::ElementType&... i_rest)
    : attr(i_attr)
    , rest(i_rest...) {}

  /**
   * Creates a list of vertices from the given data. The attribute values for each vertex are taken
   * from the given iterators, which are incremented for each vertex to be created. The iterators
   * must be given in the correct order according the type of this vertex.
   *
   * @tparam I the types of the given iterators
   * @param count the number of vertices to obtain from the given iterators
   * @param cur the attribute value iterators
   * @return the list of vertices
   */
  template <typename... I>
  static std::vector<GLVertex<AttrType, AttrTypeRest...>> toList(const size_t count, I... cur) {
    static_assert(
      sizeof...(I) == sizeof...(AttrTypeRest) + 1,
      "number of iterators must match number of vertex attributes");
    std::vector<GLVertex<AttrType, AttrTypeRest...>> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      result.emplace_back((*cur++)...);
    }
    return result;
  }
};

/**
 * Template specialization of the GLVertex template for the case of a single vertex attribute. This
 * attribute is handled in this template. This specialization is the base case for the recursive
 * GLVertex template.
 *
 * @tparam AttrType the type of the vertex attribute stored here
 */
template <typename AttrType> struct GLVertex<AttrType> {
  using Type = GLVertexType<AttrType>;

  typename AttrType::ElementType attr;

  /**
   * Creates a new vertex.
   */
  GLVertex() = default;

  // Copy and move constructors
  GLVertex(const GLVertex<AttrType>& other) = default;
  GLVertex(GLVertex<AttrType>&& other) noexcept = default;

  // Assignment operators
  GLVertex<AttrType>& operator=(const GLVertex<AttrType>& other) = default;
  GLVertex<AttrType>& operator=(GLVertex<AttrType>&& other) noexcept = default;

  // explicitly declare the following two constructors instead of using type deduction with an
  // rvalue reference to avoid any clashes with the copy / move constructors

  /**
   * Creates a new vertex by moving the given attribute value into this vertex.
   *
   * @param i_attr the value of the first attribute
   */
  explicit GLVertex(typename AttrType::ElementType&& i_attr)
    : attr(std::move(i_attr)) {}

  /**
   * Creates a new vertex by copying the given attribute value into this vertex.
   *
   * @param i_attr the value of the first attribute
   */
  explicit GLVertex(const typename AttrType::ElementType& i_attr)
    : attr(i_attr) {}

  /**
   * Creates a list of vertices from the given data. The attribute values for each vertex are taken
   * from the given iterator, which is incremented for each vertex to be created.
   *
   * @tparam I the type of the given iterator
   * @param count the number of vertices to obtain from the given iterator
   * @param cur the attribute value iterator
   * @return the list of vertices
   */
  template <typename I> static std::vector<GLVertex<AttrType>> toList(const size_t count, I cur) {
    std::vector<GLVertex<AttrType>> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      result.emplace_back(*cur++);
    }
    return result;
  }
};

/**
 * A helper to access the value of a vertex attribute. The index of the value to be accessed is
 * given as the template parameter. The struct uses recursion to access the value, and there is a
 * specialization for the base case where I becomes 0.
 *
 * @tparam I the index of the vertex attribute whose value should be accessed
 */
template <size_t I> struct GetVertexComponent {
  /**
   * Returns the value of the attribute at index I of the given vertex.
   *
   * @tparam AttrTypes the vertex attribute types
   * @param v the vertex
   * @return the value of the attribute at index I
   */
  template <typename... AttrTypes> static const auto& get(const GLVertex<AttrTypes...>& v) {
    return GetVertexComponent<I - 1>::get(v.rest);
  }

  /**
   * Returns the value of the attribute at index I of the given vertex.
   *
   * @tparam AttrTypes the vertex attribute types
   * @param v the vertex
   * @return the value of the attribute at index I
   */
  template <typename... AttrTypes> const auto& operator()(const GLVertex<AttrTypes...>& v) const {
    return get(v);
  }
};

/**
 * Specialization for the base case with index 0.
 */
template <> struct GetVertexComponent<0> {
  /**
   * Returns the value of the first attribute of the given vertex.
   *
   * @tparam AttrTypes the vertex attribute types
   * @param v the vertex
   * @return the value of the first attribute
   */
  template <typename... AttrTypes> static const auto& get(const GLVertex<AttrTypes...>& v) {
    return v.attr;
  }

  /**
   * Returns the value of the first attribute of the given vertex.
   *
   * @tparam AttrTypes the vertex attribute types
   * @param v the vertex
   * @return the value of the first attribute
   */
  template <typename... AttrTypes> const auto& operator()(const GLVertex<AttrTypes...>& v) const {
    return get(v);
  }
};

/**
 * Helper to get access a vertex attribute value by index.
 *
 * @tparam I the index of the vertex attribute
 * @tparam Attrs the vertex attribute types
 * @param v the vertex
 * @return a reference to the attribute value
 */
template <size_t I, typename... Attrs> const auto& getVertexComponent(const GLVertex<Attrs...>& v) {
  return GetVertexComponent<I>::get(v);
}
} // namespace Renderer
} // namespace TrenchBroom
