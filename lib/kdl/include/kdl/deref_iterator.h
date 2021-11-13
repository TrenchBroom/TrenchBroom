/*
 Copyright 2021 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <iterator>

namespace kdl {
template <typename I> class deref_iterator {
public:
  using iterator_category = typename I::iterator_category;
  using difference_type = typename I::difference_type;
  using value_type = std::remove_reference_t<decltype(*(std::declval<typename I::value_type>()))>;
  using reference = std::add_lvalue_reference_t<value_type>;
  using pointer = std::add_pointer_t<value_type>;

private:
  I m_it;

public:
  deref_iterator(I it)
    : m_it{it} {}

  friend bool operator<(const deref_iterator& lhs, const deref_iterator& rhs) {
    return lhs.m_it < rhs.m_it;
  }
  friend bool operator<(const deref_iterator& lhs, const I& rhs) { return lhs.m_it < rhs; }
  friend bool operator<(const I& lhs, const deref_iterator& rhs) { return lhs < rhs.m_it; }

  friend bool operator>(const deref_iterator& lhs, const deref_iterator& rhs) {
    return lhs.m_it > rhs.m_it;
  }
  friend bool operator>(const deref_iterator& lhs, const I& rhs) { return lhs.m_it > rhs; }
  friend bool operator>(const I& lhs, const deref_iterator& rhs) { return lhs > rhs.m_it; }

  friend bool operator==(const deref_iterator& lhs, const deref_iterator& rhs) {
    return lhs.m_it == rhs.m_it;
  }
  friend bool operator==(const deref_iterator& lhs, const I& rhs) { return lhs.m_it == rhs; }
  friend bool operator==(const I& lhs, const deref_iterator& rhs) { return lhs == rhs.m_it; }

  friend bool operator!=(const deref_iterator& lhs, const deref_iterator& rhs) {
    return lhs.m_it != rhs.m_it;
  }
  friend bool operator!=(const deref_iterator& lhs, const I& rhs) { return lhs.m_it != rhs; }
  friend bool operator!=(const I& lhs, const deref_iterator& rhs) { return lhs != rhs.m_it; }

  deref_iterator& operator++() {
    m_it++;
    return *this;
  }

  deref_iterator operator++(int) {
    auto result = deref_iterator(*this);
    ++m_it;
    return result;
  }

  reference operator*() const { return **m_it; }
  pointer operator->() const { return *m_it; }
};

template <typename I> deref_iterator(I) -> deref_iterator<I>;

template <typename R> class deref_range {
private:
  R& m_range;

public:
  deref_range(R& range)
    : m_range{range} {}

  auto begin() { return deref_iterator{m_range.begin()}; }

  auto end() { return deref_iterator{m_range.end()}; }

  auto begin() const { return deref_iterator{m_range.begin()}; }

  auto end() const { return deref_iterator{m_range.end()}; }

  auto cbegin() { return deref_iterator{m_range.cbegin()}; }

  auto cend() { return deref_iterator{m_range.cend()}; }

  auto rbegin() { return deref_iterator{m_range.rbegin()}; }

  auto rend() { return deref_iterator{m_range.rend()}; }

  auto rbegin() const { return deref_iterator{m_range.rbegin()}; }

  auto rend() const { return deref_iterator{m_range.rend()}; }

  auto crbegin() { return deref_iterator{m_range.crbegin()}; }

  auto crend() { return deref_iterator{m_range.crend()}; }
};

template <typename R> deref_range(R) -> deref_range<R>;

template <typename R> class const_deref_range {
private:
  const R& m_range;

public:
  const_deref_range(const R& range)
    : m_range{range} {}

  auto begin() { return deref_iterator{m_range.begin()}; }

  auto end() { return deref_iterator{m_range.end()}; }

  auto begin() const { return deref_iterator{m_range.begin()}; }

  auto end() const { return deref_iterator{m_range.end()}; }

  auto cbegin() { return deref_iterator{m_range.cbegin()}; }

  auto cend() { return deref_iterator{m_range.cend()}; }

  auto rbegin() { return deref_iterator{m_range.rbegin()}; }

  auto rend() { return deref_iterator{m_range.rend()}; }

  auto rbegin() const { return deref_iterator{m_range.rbegin()}; }

  auto rend() const { return deref_iterator{m_range.rend()}; }

  auto crbegin() { return deref_iterator{m_range.crbegin()}; }

  auto crend() { return deref_iterator{m_range.crend()}; }
};

template <typename R> const_deref_range(R) -> const_deref_range<R>;
} // namespace kdl
