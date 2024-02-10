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

#include "collection_utils.h"

// Note: all except <cassert> are included by <vector> anyway, so there's no point in
// splitting this up further
#include <algorithm> // for std::sort, std::unique, std::find, std::find_if, std::remove, std::remove_if
#include <cassert>
#include <functional> // for std::less
#include <iterator>   // std::back_inserter
#include <optional>
#include <type_traits> // for std::less
#include <vector>

namespace kdl
{
namespace detail
{
template <typename = void, typename... C>
struct has_std_size : std::false_type
{
};

template <typename C>
struct has_std_size<std::void_t<decltype(std::size(std::declval<C>()))>, C>
  : std::true_type
{
};

template <typename C>
inline constexpr bool has_std_size_v = has_std_size<void, C>::value;

template <typename = void, typename... C>
struct has_size_member : std::false_type
{
};

template <typename C>
struct has_size_member<std::void_t<decltype(std::declval<C>().size())>, C>
  : std::true_type
{
};

template <typename C>
inline constexpr bool has_size_member_v = has_size_member<void, C>::value;
} // namespace detail

template <typename V, typename C>
void vec_reserve_to(V& v, const C& c)
{
  if constexpr (detail::has_std_size_v<C>)
  {
    v.reserve(std::size_t(std::size(c)));
  }
  else if constexpr (detail::has_size_member_v<C>)
  {
    v.reserve(std::size_t(c.size()));
  }
}

template <typename T, typename... Rest>
std::vector<T> vec_from(T t, Rest... rest)
{
  auto result = std::vector<T>{};
  result.reserve(sizeof...(rest) + 1);
  result.push_back(std::move(t));
  (..., result.push_back(std::move(rest)));
  return result;
}

/**
 * Returns the vector element at the given index.
 *
 * Precondition: 0 <= index < v.size()
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 * @param index the index
 * @return a const reference to the element at the given index
 */
template <typename T, typename I>
const T& vec_at(const std::vector<T>& v, const I index)
{
  assert(index >= 0);
  const auto index_s = static_cast<typename std::vector<T>::size_type>(index);
  assert(index_s < v.size());
  return v[index_s];
}

/**
 * Returns the vector element at the given index.
 *
 * Precondition: 0 <= index < v.size()
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 * @param index the index
 * @return a const reference to the element at the given index
 */
template <typename T, typename I>
T& vec_at(std::vector<T>& v, const I index)
{
  assert(index >= 0);
  const auto index_s = static_cast<typename std::vector<T>::size_type>(index);
  assert(index_s < v.size());
  return v[index_s];
}

/**
 * Removes the last element of the given vector and returns it.
 *
 * Precondition: !v.empty()
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 * @return the last element of the given vector
 */
template <typename T>
T vec_pop_back(std::vector<T>& v)
{
  assert(!v.empty());
  T result = std::move(v.back());
  v.pop_back();
  return result;
}

/**
 * Removes the first element of the given vector and returns it.
 *
 * Precondition: !v.empty()
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 * @return the first element of the given vector
 */
template <typename T>
T vec_pop_front(std::vector<T>& v)
{
  assert(!v.empty());
  T result = std::move(v.front());
  v.erase(v.begin(), v.begin() + 1);
  return result;
}

/**
 * Returns a vector containing elements of type O, each of which is constructed by passing
 * the corresponding element of v to the constructor of o, e.g. result.push_back(O(e)),
 * where result is the resulting vector, and e is an element from v.
 *
 * Precondition: O must be constructible with an argument of type T
 *
 * @tparam O the type of the result vector elements
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @param v the vector to cast
 * @return a vector containing the elements of a, but with O as the element type
 */
template <typename O, typename T, typename A>
std::vector<O*> vec_dynamic_cast(std::vector<T*, A> v)
{
  if constexpr (std::is_same_v<T, O>)
  {
    return v;
  }
  else
  {
    auto result = std::vector<O*>{};
    result.reserve(v.size());
    for (auto& e : v)
    {
      if (auto o = dynamic_cast<O*>(e))
      {
        result.push_back(std::move(o));
      }
    }
    return result;
  }
}

/**
 * Returns a vector containing elements of type O, each of which is constructed by passing
 * the corresponding element of v to the constructor of o, e.g. result.push_back(O(e)),
 * where result is the resulting vector, and e is an element from v.
 *
 * Precondition: O must be constructible with an argument of type T
 *
 * @tparam O the type of the result vector elements
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @param v the vector to cast
 * @return a vector containing the elements of a, but with O as the element type
 */
template <typename O, typename T, typename A>
std::vector<O> vec_static_cast(std::vector<T, A> v)
{
  if constexpr (std::is_same_v<T, O>)
  {
    return v;
  }
  else
  {
    auto result = std::vector<O>{};
    result.reserve(v.size());
    for (auto& e : v)
    {
      result.push_back(static_cast<O>(e));
    }
    return result;
  }
}

/**
 * Finds the smallest index at which the given predicate is satisified in the given
 * vector. If the given vector does not such a value, an empty optional is returned.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam P the predicate type
 * @param v the vector to check
 * @param p the predicate
 * @return the smallest index at which the given predicate is satisfied in the given
 * vector or an empty optional if the given vector does not contain such a value
 */
template <
  typename T,
  typename A,
  typename P,
  typename std::enable_if_t<std::is_invocable_r_v<bool, P, const T&>>* = nullptr>
std::optional<typename std::vector<T, A>::size_type> vec_index_of(
  const std::vector<T, A>& v, P&& p)
{
  using IndexType = typename std::vector<T, A>::size_type;
  for (IndexType i = 0; i < v.size(); ++i)
  {
    if (p(v[i]))
    {
      return i;
    }
  }
  return std::nullopt;
}

/**
 * Finds the smallest index at which the given value is found in the given vector. If the
 * given vector does not contain the given value, the size of the vector is returned.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam X the value type
 * @param v the vector to check
 * @param x the value to find
 * @return the smallest index at which the given value is found in the given vector or the
 * vector's size if the given vector does not contain the given value
 */
template <typename T, typename A, typename X>
std::optional<typename std::vector<T, A>::size_type> vec_index_of(
  const std::vector<T, A>& v, const X& x)
{
  return vec_index_of(v, [&](const auto& e) { return e == x; });
}

/**
 * Finds the smallest index at which the given predicate is satisified in the given
 * vector. If the given vector does not such a value, an empty optional is returned.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam P the predicate type
 * @param v the vector to check
 * @param p the predicate
 * @return true if the given vector contains an element that satisfies the given predicate
 */
template <
  typename T,
  typename A,
  typename P,
  typename std::enable_if_t<std::is_invocable_r_v<bool, P, const T&>>* = nullptr>
bool vec_contains(const std::vector<T, A>& v, P&& p)
{
  return vec_index_of(v, std::forward<P>(p)).has_value();
}

/**
 * Checks if the given value is contained in the given vector.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam X the value type
 * @param v the vector to check
 * @param x the value to check
 * @return true if the given vector contains the given value and false otherwise
 */
template <typename T, typename A, typename X>
bool vec_contains(const std::vector<T, A>& v, const X& x)
{
  return vec_index_of(v, x).has_value();
}

namespace detail
{
template <typename T, typename A>
void vec_concat(std::vector<T, A>&)
{
}

template <typename T, typename A, typename Arg>
void vec_concat(std::vector<T, A>& v1, const Arg& arg)
{
  v1.insert(std::end(v1), std::begin(arg), std::end(arg));
}

template <typename T, typename A, typename Arg, typename... Rest>
void vec_concat(std::vector<T, A>& v1, const Arg& arg, Rest&&... rest)
{
  vec_concat(v1, arg);
  vec_concat(v1, std::forward<Rest>(rest)...);
}

template <typename T, typename A, typename Arg>
void vec_concat(std::vector<T, A>& v1, Arg&& arg)
{
  for (auto& x : arg)
  {
    v1.push_back(std::move(x));
  }
}

template <typename T, typename A, typename Arg, typename... Rest>
void vec_concat(std::vector<T, A>& v1, Arg&& arg, Rest&&... rest)
{
  vec_concat(v1, std::forward<Arg>(arg));
  vec_concat(v1, std::forward<Rest>(rest)...);
}
} // namespace detail

/**
 * Concatenates the given vectors. Each element of function argument pack args must be a
 * vector with value_type T and allocator A. If a vector is passed by rvalue reference,
 * its elements will be moved into the result, otherwise they will be copied.
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @tparam Args parameter pack containing the vectors to append to v
 * @param v the first vector to concatenate
 * @param args the remaining vectors to concatenate
 */
template <typename T, typename A, typename... Args>
std::vector<T, A> vec_concat(std::vector<T, A> v, Args... args)
{
  v.reserve(kdl::col_total_size(v, args...));
  detail::vec_concat(v, std::move(args)...);
  return v;
}

/**
 * Appends the given elements. Each element must be a type convertible to T and it is
 * perfectly forwarded to std::vector<T, A>::push_back.
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @tparam Args parameter pack containing the elements to append to v
 * @param v the first vector to append to
 * @param args the elements to append
 */
template <typename T, typename A, typename... Args>
auto vec_push_back(std::vector<T, A> v, Args... args)
{
  v.reserve(v.size() + sizeof...(args));
  (..., v.push_back(std::forward<Args>(args)));
  return v;
}

/**
 * Returns a slice of the given vector starting at offset and with count elements.
 *
 * The elements are copied into the returned vector.
 *
 * Precondition: offset + count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a slice of
 * @param offset the offset of the first element to return
 * @param count the number of elements to return
 * @return a vector containing the slice of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice(
  const std::vector<T, A>& v, const std::size_t offset, const std::size_t count)
{
  assert(offset + count <= v.size());

  std::vector<T, A> result;
  result.reserve(count);

  for (std::size_t i = 0u; i < count; ++i)
  {
    result.push_back(v[i + offset]);
  }

  return result;
}

/**
 * Returns a slice of the given vector starting at offset and with count elements.
 *
 * The elements are moved into the returned vector.
 *
 * Precondition: offset + count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a slice of
 * @param offset the offset of the first element to return
 * @param count the number of elements to return
 * @return a vector containing the slice of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice(
  std::vector<T, A>&& v, const std::size_t offset, const std::size_t count)
{
  assert(offset + count <= v.size());

  std::vector<T, A> result;
  result.reserve(count);

  for (std::size_t i = 0u; i < count; ++i)
  {
    result.push_back(std::move(v[i + offset]));
  }

  return result;
}

/**
 * Returns a prefix of the given vector with count elements.
 *
 * The elements are copied into the returned vector.
 *
 * Precondition: count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a prefix of
 * @param count the number of elements to return
 * @return a vector containing the prefix of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice_prefix(const std::vector<T, A>& v, const std::size_t count)
{
  assert(count <= v.size());
  return vec_slice(v, 0u, count);
}

/**
 * Returns a prefix of the given vector with count elements.
 *
 * The elements are moved into the returned vector.
 *
 * Precondition: count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a prefix of
 * @param count the number of elements to return
 * @return a vector containing the prefix of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice_prefix(std::vector<T, A>&& v, const std::size_t count)
{
  assert(count <= v.size());
  return vec_slice(std::move(v), 0u, count);
}

/**
 * Returns a suffix of the given vector with count elements.
 *
 * The elements are copied into the returned vector.
 *
 * Precondition: count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a prefix of
 * @param count the number of elements to return
 * @return a vector containing the prefix of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice_suffix(const std::vector<T, A>& v, const std::size_t count)
{
  assert(count <= v.size());
  return vec_slice(v, v.size() - count, count);
}

/**
 * Returns a suffix of the given vector with count elements.
 *
 * The elements are moved into the returned vector.
 *
 * Precondition: count does not exceed the number of elements in the given vector
 *
 * @tparam T the element type
 * @tparam A the allocator type
 * @param v the vector to return a prefix of
 * @param count the number of elements to return
 * @return a vector containing the prefix of the given vector
 */
template <typename T, typename A>
std::vector<T, A> vec_slice_suffix(std::vector<T, A>&& v, const std::size_t count)
{
  assert(count <= v.size());
  return vec_slice(std::move(v), v.size() - count, count);
}

/**
 * Erases every element from the given vector which is equal to the given value using the
 * erase-remove idiom. Returns a vector with the remaining elements.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam X the value type
 * @param v the vector
 * @param x the value to erase
 * @return a vector with the remaining elements
 */
template <typename T, typename A, typename X>
std::vector<T, A> vec_erase(std::vector<T, A> v, const X& x)
{
  v.erase(std::remove(std::begin(v), std::end(v), x), std::end(v));
  return v;
}

/**
 * Erases every element from the given vector for which the given predicate evaluates to
 * true using the erase-remove idiom. Returns a vector with the remaining elements.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam P the predicate type
 * @param v the vector
 * @param predicate the predicate
 * @return a vector with the remaining elements
 */
template <typename T, typename A, typename P>
std::vector<T, A> vec_erase_if(std::vector<T, A> v, const P& predicate)
{
  v.erase(std::remove_if(std::begin(v), std::end(v), predicate), std::end(v));
  return v;
}

/**
 * Erases the element at the given index from the given vector. The element is swapped
 * with the last element of the vector, and then the last element is erased. Returns a
 * vector with the remaining elements.
 *
 * Precondition: i < v.size()
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @param v the vector
 * @param i the index of the element to erase, which must be less than the given vector's
 * size
 * @return a vector with the remaining elements
 */
template <typename T, typename A>
std::vector<T, A> vec_erase_at(
  std::vector<T, A> v, const typename std::vector<T, A>::size_type i)
{
  assert(i < v.size());
  auto it =
    std::next(std::begin(v), static_cast<typename std::vector<T, A>::difference_type>(i));
  v.erase(it);
  return v;
}

/**
 * Erases every value from the given vector which is equal to any value in the given
 * collection. Returns a vector with the remaining elements.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam C the collection type
 * @param v the vector to erase elements from
 * @param c the collection of values to erase
 * @return a vector with the remaining elements
 */
template <typename T, typename A, typename C>
std::vector<T, A> vec_erase_all(std::vector<T, A> v, const C& c)
{
  for (const auto& x : c)
  {
    v = vec_erase(std::move(v), x);
  }
  return v;
}

/**
 * Sorts the elements of the given vector according to the given comparator.
 * Returns a vector with the sorted elements.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam Compare the type of the comparator to use
 * @param v the vector to sort
 * @param cmp the comparator to use for comparisons
 * @return a vector with the sorted elements
 */
template <typename T, typename A, typename Compare = std::less<T>>
std::vector<T, A> vec_sort(std::vector<T, A> v, const Compare& cmp = Compare())
{
  std::sort(std::begin(v), std::end(v), cmp);
  return v;
}

/**
 * Sorts the elements of the given vector and removes all duplicate values. A value is a
 * duplicate if it is equivalent to its predecessor in the vector. Returns a vector with
 * the remaining sorted elements.
 *
 * @tparam T the type of the vector elements
 * @tparam A the vector's allocator type
 * @tparam Compare the type of the comparator to use
 * @param v the vector to sort and remove duplicates from
 * @param cmp the comparator to use for sorting and for determining equivalence
 * @return a vector with the remaining sorted elements
 */
template <typename T, typename A, typename Compare = std::less<T>>
std::vector<T, A> vec_sort_and_remove_duplicates(
  std::vector<T, A> v, const Compare& cmp = Compare())
{
  std::sort(std::begin(v), std::end(v), cmp);
  v.erase(
    std::unique(std::begin(v), std::end(v), kdl::equivalence<T, Compare>(cmp)),
    std::end(v));
  return v;
}

/**
 * Returns a vector containing every element of the given range that satisfies the given
 * predicate. The elements are moved into the returned vector in the same order as they
 * are in the given range.
 *
 * @tparam Range the type of the range
 * @tparam Predicate the type of the predicate to apply, must be of type `bool(const T&)`
 * @tparam T the type of the vector elements
 * @param range the range
 * @param predicate the predicate to apply
 * @return a vector containing the elements that passed the predicate
 */
template <
  typename Range,
  typename Predicate,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Predicate, const T&>>* = nullptr>
auto vec_filter(Range range, Predicate&& predicate)
{
  auto result = std::vector<T>{};
  vec_reserve_to(result, range);

  for (auto& x : range)
  {
    if (predicate(x))
    {
      result.push_back(std::move(x));
    }
  }

  return result;
}

/**
 * Returns a vector containing every element of the given range that passes the given
 * predicate. The elements are moved into the returned vector in the same order as they
 * are in the given range.
 *
 * This version passes the element indices to the predicate function.
 *
 * @tparam Range the type of the range
 * @tparam Predicate the type of the predicate to apply, must be of type `bool(const T&,
 * size_t)`
 * @tparam T the type of the vector elements
 * @param range the range
 * @param predicate the predicate to apply
 * @return a vector containing the elements that passed the predicate
 */
template <
  typename Range,
  typename Predicate,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Predicate, const T&, std::size_t>>* =
    nullptr>
auto vec_filter(Range range, Predicate&& predicate)
{
  auto result = std::vector<T>{};
  vec_reserve_to(result, range);

  for (std::size_t i = 0u; i < std::size(range); ++i)
  {
    if (predicate(range[i], i))
    {
      result.push_back(std::move(range[i]));
    }
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in r.
 *
 * The elements are passed to the given transformation as const lvalue references.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(const
 * T&)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, const T&>>* = nullptr>
auto vec_transform(const Range& range, Transform&& transform)
{
  using ResultType = decltype(transform(std::declval<const T&>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (const auto& x : range)
  {
    result.push_back(transform(x));
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in the range.
 *
 * The elements are passed to the given transformation as const lvalue references.
 *
 * This version passes the element indices to the transformation.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(const
 * T&, std::size_t)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, const T&, std::size_t>>* =
    nullptr>
auto vec_transform(const Range& range, Transform&& transform)
{
  using ResultType =
    decltype(transform(std::declval<const T&>(), std::declval<std::size_t>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (std::size_t i = 0u; i < std::size(range); ++i)
  {
    result.push_back(transform(range[i], i));
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in r.
 *
 * The elements are passed to the given transformation as lvalue references.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(T&)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, T&>>* = nullptr>
auto vec_transform(Range& range, Transform&& transform)
{
  using ResultType = decltype(transform(std::declval<T&>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (auto& x : range)
  {
    result.push_back(transform(x));
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in the range.
 *
 * The elements are passed to the given transformation as lvalue references.
 *
 * This version passes the element indices to the transformation.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(T&,
 * std::size_t)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, T&, std::size_t>>* = nullptr>
auto vec_transform(Range& range, Transform&& transform)
{
  using ResultType = decltype(transform(std::declval<T&>(), std::declval<std::size_t>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (std::size_t i = 0u; i < std::size(range); ++i)
  {
    result.push_back(transform(range[i], i));
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in r.
 *
 * The elements are passed to the given transformation as rvalue references.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(T&&)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, T&&>>* = nullptr>
auto vec_transform(Range&& range, Transform&& transform)
{
  using ResultType = decltype(transform(std::declval<T&&>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (auto&& x : range)
  {
    result.push_back(transform(std::move(x)));
  }

  return result;
}

/**
 * Applies the given transformation to each element of the given range and returns a
 * vector containing the resulting values, in order in which their original elements
 * appeared in the range.
 *
 * The elements are passed to the given transformation as rvalue references.
 *
 * This version passes the element indices to the transformation.
 *
 * @tparam Range the type of the range
 * @tparam Transform the type of the transformation to apply, must be of type `auto(T&&,
 * std::size_t)`
 * @tparam T the type of the elements
 * @param range the range
 * @param transform the transformation to apply
 * @return a vector containing the transformed values
 */
template <
  typename Range,
  typename Transform,
  typename T = typename Range::value_type,
  typename std::enable_if_t<std::is_invocable_v<Transform, T&&, std::size_t>>* = nullptr>
auto vec_transform(Range&& range, Transform&& transform)
{
  using ResultType =
    decltype(transform(std::declval<T&&>(), std::declval<std::size_t>()));

  auto result = std::vector<ResultType>{};
  vec_reserve_to(result, range);

  for (std::size_t i = 0u; i < std::size(range); ++i)
  {
    result.push_back(transform(std::move(range[i]), i));
  }

  return result;
}

/**
 * Given a vector of nested vectors, this function returns a vector that contains the
 * elements of the nested vectors.
 *
 * The resulting vector contains the elements of the given vector in the order in which
 * they appear.
 *
 * @tparam T the element type
 * @tparam A1 the nested vector allocator type
 * @tparam A2 the outer vector allocator type
 * @param vec the vector to flatten
 * @result the flattened vector
 */
template <typename T, typename A1, typename A2>
auto vec_flatten(std::vector<std::vector<T, A1>, A2> vec)
{
  std::size_t totalSize = 0u;
  for (const auto& nested : vec)
  {
    totalSize += nested.size();
  }

  auto result = std::vector<T, A1>{};
  result.reserve(totalSize);

  for (auto& nested : vec)
  {
    result = vec_concat(std::move(result), std::move(nested));
  }

  return result;
}

/**
 * Returns a vector containing those values from s1 which are not also in s2. Values from
 * s1 and s2 are compared using the common comparator from both sets.
 *
 * Expects that both S1 and S2 declare the types of their values with ::value_type and
 * and that S1 and S2 are sorted and unique according to comparator C.
 *
 * The value type of the returned vector is the common type of S1 and S2's member types.
 * The values from s1 which are not also in s2 are added to the returned vector in the
 * order in which they appear in s2.
 *
 * @tparam S1 the type of the first set
 * @tparam S2 the type of the second set
 * @tparam C the type of the comparator to use
 * @param s1 the first set
 * @param s2 the second set
 * @param c the comparator to use
 * @return a vector containing the set difference of s1 and s2.
 */
template <
  typename S1,
  typename S2,
  typename C =
    std::less<std::common_type_t<typename S1::value_type, typename S2::value_type>>>
auto set_difference(const S1& s1, const S2& s2, const C& c = C{})
{
  using T1 = typename S1::value_type;
  using T2 = typename S2::value_type;
  using T = std::common_type_t<T1, T2>;

  std::vector<T> result;
  result.reserve(s1.size());
  std::set_difference(
    std::begin(s1),
    std::end(s1),
    std::begin(s2),
    std::end(s2),
    std::back_inserter(result),
    c);
  return result;
}

/**
 * Returns a vector containing all values from s1 and s2 without duplicates. A pair of
 * values from s1 and s2 is a duplicate if the values are equivalent according to the
 * common comparator of s1 and s2.
 *
 * Expects that both S1 and S2 declare the types of their values with ::value_type and
 * and that S1 and S2 are sorted and unique according to comparator C.
 *
 * The value type of the returned vector is the common type of S1 and S2's member types.
 * The order of the values in the returned vector complies with the common comparator of
 * s1 and s2.
 *
 * @tparam S1 the type of the first set
 * @tparam S2 the type of the second set
 * @tparam C the type of the comparator to use
 * @param s1 the first set
 * @param s2 the second set
 * @param c the comparator to use
 * @return a vector containing the set union of s1 and s2.
 */
template <
  typename S1,
  typename S2,
  typename C =
    std::less<std::common_type_t<typename S1::value_type, typename S2::value_type>>>
auto set_union(const S1& s1, const S2& s2, const C& c = C{})
{
  using T1 = typename S1::value_type;
  using T2 = typename S2::value_type;
  using T = typename std::common_type<T1, T2>::type;

  std::vector<T> result;
  result.reserve(s1.size() + s2.size());
  std::set_union(
    std::begin(s1),
    std::end(s1),
    std::begin(s2),
    std::end(s2),
    std::back_inserter(result),
    c);
  return result;
}

/**
 * Returns a vector containing all values from s1 and s2 which are present in both sets.
 *
 * Expects that both S1 and S2 declare the types of their values with ::value_type and
 * and that S1 and S2 are sorted and unique according to comparator C.
 *
 * The value type of the returned vector is the common type of S1 and S2's member types.
 * The order of the values in the returned vector complies with the common comparator of
 * s1 and s2.
 *
 * @tparam S1 the type of the first set
 * @tparam S2 the type of the second set
 * @tparam C the type of the comparator to use
 * @param s1 the first set
 * @param s2 the second set
 * @param c the comparator to use
 * @return a vector containing the set union of s1 and s2.
 */
template <
  typename S1,
  typename S2,
  typename C =
    std::less<std::common_type_t<typename S1::value_type, typename S2::value_type>>>
auto set_intersection(const S1& s1, const S2& s2, const C& c = C{})
{
  using T1 = typename S1::value_type;
  using T2 = typename S2::value_type;
  using T = typename std::common_type<T1, T2>::type;

  std::vector<T> result;
  result.reserve(s1.size() + s2.size());
  std::set_intersection(
    std::begin(s1),
    std::end(s1),
    std::begin(s2),
    std::end(s2),
    std::back_inserter(result),
    c);
  return result;
}

/**
 * Checks if the given sets have a shared element.
 *
 * @tparam S1 the type of the first set
 * @tparam S2 the type of the second set
 * @tparam C the comparator to use when comparing set elements
 * @param s1 the first set
 * @param s2 the second set
 * @return true if the given sets have a shared element and false otherwise
 */
template <
  typename S1,
  typename S2,
  typename C =
    std::less<std::common_type_t<typename S1::value_type, typename S2::value_type>>>
auto set_has_shared_element(const S1& s1, const S2& s2, const C& cmp = C{})
{
  auto it1 = std::begin(s1);
  auto it2 = std::begin(s2);
  while (it1 != std::end(s1) && it2 != std::end(s2))
  {
    if (cmp(*it1, *it2))
    {
      ++it1;
    }
    else if (cmp(*it2, *it1))
    {
      ++it2;
    }
    else
    {
      return true;
    }
  }

  return false;
}

/**
 * Clears the given vector and ensures that it has a capacity of 0 afterwards.
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 */
template <typename T>
void vec_clear_to_zero(std::vector<T>& v)
{
  v.clear();
  v.shrink_to_fit();
}

/**
 * Applies the given deleter to every element of the given vector, and clears the vector
 * afterwards.
 *
 * @tparam T the type of the vector elements
 * @param v the vector
 */
template <typename T, typename D = deleter<T*>>
void vec_clear_and_delete(std::vector<T*>& v, const D& deleter = D())
{
  kdl::col_delete_all(v, deleter);
  v.clear();
}
} // namespace kdl
