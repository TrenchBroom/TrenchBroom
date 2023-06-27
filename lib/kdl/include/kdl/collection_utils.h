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

#include <algorithm>   // for std::remove
#include <functional>  // for std::less
#include <type_traits> // for std::is_convertible

namespace kdl
{
template <typename T>
struct deleter
{
  void operator()(T ptr) const { delete ptr; }
};

template <typename First>
auto combine_cmp(First first)
{
  return first;
}

/**
 * Returns a comparator that applies the given comparators in lexicographical order.
 *
 * @tparam First the type of the first comparator
 * @tparam Rest the types of the remaining comparators
 * @param first the first comparator
 * @param rest the remaining comparators
 */
template <typename First, typename... Rest>
auto combine_cmp(First first, Rest... rest)
{
  return [first = std::move(first), rest = std::move(rest...)](
           const auto& lhs, const auto& rhs) {
    return first(lhs, rhs)   ? true
           : first(rhs, lhs) ? false
                             : combine_cmp(std::move(rest))(lhs, rhs);
  };
}

/**
 * Provides a notion of equivalence using a comparator. Two values are equivalent if they
 * are mutually incomparable by means of the comparator.
 *
 * @tparam T the value type
 * @tparam Compare the type of the comparator, defaults to std::less<T>
 */
template <typename T, typename Compare = std::less<T>>
struct equivalence
{
  Compare cmp;

  explicit equivalence(const Compare& i_cmp = Compare())
    : cmp(i_cmp)
  {
  }

  bool operator()(const T& lhs, const T& rhs) const
  {
    return !cmp(lhs, rhs) && !cmp(rhs, lhs);
  }
};

/**
 * Removes every element in range [first2, last2) from range [first1, last1). Removal is
 * done by shifting the removed elements to the end of the range [first1, last1) in such a
 * way that the range [first1, result) contains the retained elements and [result, last1)
 * contains the removed elements. The relative order of the elements in reach of the two
 * resulting ranges is retained.
 *
 * @tparam I1 the type of the iterators of the range to remove from
 * @tparam I2 the type of the iterators of the range to remove
 * @param first1 the start of the range to remove from
 * @param last1 the end of the range to remove from (past-the-end iterator)
 * @param first2 the start of the range to remove
 * @param last2 the end of the range to remove (past-the-end iterator)
 * @return the position of the first removed element in the range [first1, last1) such
 * that [first1, result) contains the retained elements and [result, last1) contains the
 * removed elements
 */
template <typename I1, typename I2>
I1 range_remove_all(I1 first1, I1 last1, I2 first2, I2 last2)
{
  I1 result = last1;
  while (first2 != last2)
  {
    result = std::remove(first1, result, *first2++);
  }
  return result;
}

/**
 * Applies the given deleter to all elements in range [first, last).
 *
 * @tparam I the iterator type
 * @tparam D the deleter type, defaults to Deleter
 * @param first the start of the range of values to delete
 * @param last the end of the range of values to delete (past-the-end iterator)
 * @param deleter the deleter to apply
 */
template <typename I, typename D = deleter<typename I::value_type>>
void range_delete_all(I first, I last, const D& deleter = D())
{
  while (first != last)
  {
    deleter(*first++);
  }
}

/**
 * Performs lexicographical comparison of the given ranges [first1, last1) and [first2,
 * last2) using the given comparator. Returns -1 if the first range is less than the
 * second range, or +1 in the opposite case, or 0 if both ranges are equivalent.
 *
 * @tparam I1 the iterator type of the first range
 * @tparam I2 the iterator type of the second range
 * @tparam Compare the comparator type, defaults to std::less<T>, where T both
 * I1::value_type and I2::value_type must be convertible to T
 * @param first1 the start iterator of the first range
 * @param last1 the end iterator of the first range (past-the-end iterator)
 * @param first2 the start iterator of the second range
 * @param last2 the end iterator of the second range (past-the-end iterator)
 * @param cmp the comparator to use
 * @return an int indicating the result of the comparison
 */
template <
  typename I1,
  typename I2,
  typename Compare = std::less<
    typename std::common_type<typename I1::value_type, typename I2::value_type>::type>>
int range_lexicographical_compare(
  I1 first1, I1 last1, I2 first2, I2 last2, const Compare& cmp = Compare())
{
  while (first1 != last1 && first2 != last2)
  {
    if (cmp(*first1, *first2))
    {
      return -1;
    }
    else if (cmp(*first2, *first1))
    {
      return 1;
    }
    else
    {
      ++first1;
      ++first2;
    }
  }

  if (first1 != last1)
  {
    return 1;
  }
  else if (first2 != last2)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

/**
 * Checks whether the given ranges [first1, last1) and [first2, last2) are equivalent
 * according to using the given comparator. Two ranges are considered equivalent according
 * to a comparator if the ranges have the same number of elements, and each pair of
 * corresponding elements of the ranges is equivalent.
 *
 * @tparam I1 the iterator type of the first range
 * @tparam I2 the iterator type of the second range
 * @tparam Compare the comparator type, defaults to std::less<T>, where T both
 * I1::value_type and I2::value_type must be convertible to T
 * @param first1 the start iterator of the first range
 * @param last1 the end iterator of the first range (past-the-end iterator)
 * @param first2 the start iterator of the second range
 * @param last2 the end iterator of the second range (past-the-end iterator)
 * @param cmp the comparator to use
 * @return true if the given ranges are equivalent and false otherwise
 */
template <
  typename I1,
  typename I2,
  typename Compare = std::less<
    typename std::common_type<typename I1::value_type, typename I2::value_type>::type>>
bool range_is_equivalent(
  I1 first1, I1 last1, I2 first2, I2 last2, const Compare& cmp = Compare())
{
  // if the given iterators are random access iterators, short circuit the evaluation if
  // the sizes of the given ranges differ
  if constexpr (
    std::is_convertible_v<
      typename std::iterator_traits<I1>::iterator_category,
      std::
        random_access_iterator_tag> && std::is_convertible_v<typename std::iterator_traits<I2>::iterator_category, std::random_access_iterator_tag>)
  {
    if (last1 - first1 != last2 - first2)
    {
      return false;
    }
  }
  return kdl::range_lexicographical_compare(first1, last1, first2, last2, cmp) == 0;
}

/**
 * Returns the size of the given container cast to the given type.
 *
 * @tparam C the container type
 * @tparam O the type of the result, e.g. int
 * @param c the collection
 * @return the size of the given collection
 */
template <typename O, typename C>
O col_size(const C& c)
{
  return static_cast<O>(c.size());
}

/**
 * Computes the sum of the sizes of the given containers.
 *
 * @tparam C the type of the first container
 * @tparam Args the type of the remaining containers
 * @param c the first container
 * @param args the remaining containers
 * @return the sum of the sizes of the given containers
 */
template <typename C, typename... Args>
auto col_total_size(const C& c, Args&&... args)
{
  return (c.size() + ... + args.size());
}

/**
 * Applies the given deleter to all elements the given container.
 *
 * @tparam C the container type
 * @tparam D the deleter type, defaults to deleter
 * @param c the container
 * @param deleter the deleter to apply
 */
template <typename C, typename D = deleter<typename C::value_type>>
void col_delete_all(C& c, const D& deleter = D())
{
  kdl::range_delete_all(std::begin(c), std::end(c), deleter);
}

/**
 * Performs lexicographical comparison of the given collections c1 and c2 using the given
 * comparator. Returns -1 if the first collection is less than the second collection, or
 * +1 in the opposite case, or 0 if both collections are equivalent.
 *
 * @tparam C1 the type of the first collection
 * @tparam C2 the type of the second collection
 * @tparam Compare the comparator type, defaults to std::less<T>, where T both
 * C1::value_type and C2::value_type must be convertible to T
 * @param c1 the first collection
 * @param c2 the second collection
 * @param cmp the comparator to use
 * @return an int indicating the result of the comparison
 */
template <
  typename C1,
  typename C2,
  typename Compare = std::less<
    typename std::common_type<typename C1::value_type, typename C2::value_type>::type>>
int col_lexicographical_compare(
  const C1& c1, const C2& c2, const Compare& cmp = Compare())
{
  return kdl::range_lexicographical_compare(
    std::begin(c1), std::end(c1), std::begin(c2), std::end(c2), cmp);
}

/**
 * Checks whether the given collections are equivalent according to using the given
 * comparator. Two collections are considered equivalent according to a comparator if they
 * have the same number of elements, and each pair of corresponding elements of the
 * collections is equivalent.
 *
 * @tparam C1 the type of the first collection
 * @tparam C2 the type of the second collection
 * @tparam Compare the comparator type, defaults to std::less<T>, where T both
 * C1::value_type and C2::value_type must be convertible to T
 * @param c1 the first collection
 * @param c2 the second collection
 * @param cmp the comparator to use
 * @return true if the given collections are equivalent and false otherwise
 */
template <
  typename C1,
  typename C2,
  typename Compare = std::less<
    typename std::common_type<typename C1::value_type, typename C2::value_type>::type>>
bool col_is_equivalent(const C1& c1, const C2& c2, const Compare& cmp = Compare())
{
  if (c1.size() != c2.size())
  {
    return false;
  }
  else
  {
    return kdl::range_is_equivalent(
      std::begin(c1), std::end(c1), std::begin(c2), std::end(c2), cmp);
  }
}

/**
 * Sorts the elements of the given collection according to the given comparator.
 *
 * @tparam C the collection type
 * @tparam Compare the type of the comparator to use
 * @param c the collection to sort
 * @param cmp the comparator to use for comparisons
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
C col_sort(C c, const Compare& cmp = Compare())
{
  std::sort(std::begin(c), std::end(c), cmp);
  return c;
}

template <typename C, typename P>
bool none_of(const C& c, const P& p)
{
  return std::none_of(std::begin(c), std::end(c), p);
}

template <typename C, typename P>
bool any_of(const C& c, const P& p)
{
  return std::any_of(std::begin(c), std::end(c), p);
}

template <typename C, typename P>
bool all_of(const C& c, const P& p)
{
  return std::all_of(std::begin(c), std::end(c), p);
}

} // namespace kdl
