/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace kdl
{
/**
 * Wraps an iterator and applies a transformation function and returns its result every
 * time it is dereferenced.
 *
 * @tparam I the type of the wrapped iterator
 * @tparam Transform the type of the transformation to apply
 */
template <typename I, typename Transform>
class transform_iterator
{
public:
  using iterator_category = typename I::iterator_category;
  using value_type = std::invoke_result_t<Transform, typename I::value_type>;
  using difference_type = typename I::difference_type;
  using pointer = value_type*;
  using reference = value_type&;

private:
  I m_iter;
  Transform m_transform;

public:
  /**
   * Creates a new transform iterator with the given wrapped iterator and transformation
   * function.
   *
   * @param iter the wrapped iterator
   * @param transform the transformation function, must be movable
   */
  transform_iterator(I iter, Transform transform)
    : m_iter(iter)
    , m_transform(std::move(transform))
  {
  }

  bool operator<(const transform_iterator& other) const { return m_iter < other.m_iter; }
  bool operator>(const transform_iterator& other) const { return m_iter > other.m_iter; }
  bool operator==(const transform_iterator& other) const
  {
    return m_iter == other.m_iter;
  }
  bool operator!=(const transform_iterator& other) const
  {
    return m_iter != other.m_iter;
  }

  // prefix
  transform_iterator& operator++()
  {
    ++m_iter;
    return *this;
  }
  transform_iterator& operator--()
  {
    --m_iter;
    return *this;
  }

  // postfix
  transform_iterator operator++(int)
  {
    transform_iterator result(*this);
    ++m_iter;
    return result;
  }
  transform_iterator operator--(int)
  {
    transform_iterator result(*this);
    --m_iter;
    return result;
  }

  value_type operator*() { return m_transform(*m_iter); }
};

/**
 * Wraps a const reference to a collection, and provides transform_iterators which expose
 * the underlying collection's elements transformed by a transformation function on every
 * iterator dereference.
 *
 * @see transform_iterator
 *
 * @tparam C the collection to wrap
 * @tparam Transform the transformation function to apply to the collection's element when
 * iterators are dereferenced
 */
template <typename C, typename Transform>
class transform_adapter
{
public:
  using const_iterator = transform_iterator<typename C::const_iterator, Transform>;
  using const_reverse_iterator =
    transform_iterator<typename C::const_reverse_iterator, Transform>;

private:
  const C& m_container;
  const Transform m_transform;

public:
  /**
   * Creates a new wrapper for the given container and transformation function.
   *
   * @param container the container to adapt
   * @param transform the transformation function, must be movable
   */
  explicit transform_adapter(const C& container, Transform transform)
    : m_container(container)
    , m_transform(std::move(transform))
  {
  }

  /**
   * Indicates whether the container is empty.
   */
  bool empty() const { return m_container.empty(); }

  /**
   * Returns the number of elements in the container.
   */
  std::size_t size() const { return m_container.size(); }

  /**
   * Returns an iterator to the first element of the container. If the container is empty,
   * the returned iterator will be equal to end().
   */
  const_iterator begin() const { return cbegin(); }

  /**
   * Returns an iterator to the element following the last element of the container.
   */
  const_iterator end() const { return cend(); }

  /**
   * Returns an iterator to the first element of the container. If the container is empty,
   * the returned iterator will be equal to end().
   */
  const_iterator cbegin() const
  {
    return const_iterator(std::cbegin(m_container), m_transform);
  }

  /**
   * Returns an iterator to the element following the last element of the container.
   */
  const_iterator cend() const
  {
    return const_iterator(std::cend(m_container), m_transform);
  }

  /**
   * Returns a reverse iterator to the first element of the reverse container. If the
   * container is empty, the returned iterator is equal to rend().
   */
  const_reverse_iterator rbegin() const { return crbegin(); }

  /**
   * Returns a reverse iterator to the element following the last element of the reverse
   * container.
   */
  const_reverse_iterator rend() const { return crend(); }

  /**
   * Returns a reverse iterator to the first element of the reverse container. If the
   * container is empty, the returned iterator is equal to rend().
   */
  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(std::crbegin(m_container), m_transform);
  }

  /**
   * Returns a reverse iterator to the element following the last element of the reverse
   * container.
   */
  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(std::crend(m_container), m_transform);
  }
};

/**
 * Deduction guide.
 */
template <typename C, typename L>
transform_adapter(const C& container, L lambda) -> transform_adapter<C, L>;
} // namespace kdl
