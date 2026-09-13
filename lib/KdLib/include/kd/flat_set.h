/*
 Copyright (C) 2026 Kristian Duske

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

#include "kd/flat_set_forward.h"
#include "kd/sorted_unique.h"
#include "kd/traits.h"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace kdl
{

// A backport of std::flat_set (C++23).
//
// default template arguments are defined in flat_set_forward.h
template <typename Key, typename Compare, typename KeyContainer>
class flat_set
{
public:
  using key_type = Key;
  using value_type = Key;
  using key_compare = Compare;
  using value_compare = Compare;
  using container_type = KeyContainer;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = typename KeyContainer::size_type;
  using difference_type = typename KeyContainer::difference_type;
  using iterator = typename KeyContainer::const_iterator;
  using const_iterator = typename KeyContainer::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static_assert(
    std::is_same_v<Key, typename KeyContainer::value_type>,
    "flat_set's Key must be the same type as KeyContainer::value_type");

private:
  KeyContainer m_data;
  Compare m_cmp;

public:
  flat_set() = default;

  explicit flat_set(const key_compare& comp)
    : m_cmp{comp}
  {
  }

  explicit flat_set(container_type cont, const key_compare& comp = key_compare{})
    : m_data{std::move(cont)}
    , m_cmp{comp}
  {
    sort_and_dedup();
  }

  flat_set(sorted_unique_t, container_type cont, const key_compare& comp = key_compare{})
    : m_data{std::move(cont)}
    , m_cmp{comp}
  {
    assert(is_sorted_unique());
  }

  template <typename InputIt>
  flat_set(InputIt first, InputIt last, const key_compare& comp = key_compare{})
    : m_data(first, last)
    , m_cmp{comp}
  {
    sort_and_dedup();
  }

  template <typename InputIt>
  flat_set(
    sorted_unique_t, InputIt first, InputIt last, const key_compare& comp = key_compare{})
    : m_data(first, last)
    , m_cmp{comp}
  {
    assert(is_sorted_unique());
  }

  flat_set(
    std::initializer_list<value_type> init, const key_compare& comp = key_compare{})
    : flat_set{init.begin(), init.end(), comp}
  {
  }

  flat_set(
    sorted_unique_t,
    std::initializer_list<value_type> init,
    const key_compare& comp = key_compare{})
    : flat_set{sorted_unique, init.begin(), init.end(), comp}
  {
  }

  flat_set& operator=(std::initializer_list<value_type> init)
  {
    m_data = init;
    sort_and_dedup();
    return *this;
  }

  iterator begin() const noexcept { return m_data.begin(); }

  const_iterator cbegin() const noexcept { return m_data.cbegin(); }

  iterator end() const noexcept { return m_data.end(); }

  const_iterator cend() const noexcept { return m_data.cend(); }

  reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }

  const_reverse_iterator crbegin() const noexcept
  {
    return const_reverse_iterator{cend()};
  }

  reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

  const_reverse_iterator crend() const noexcept
  {
    return const_reverse_iterator{cbegin()};
  }

  bool empty() const noexcept { return m_data.empty(); }

  size_type size() const noexcept { return m_data.size(); }

  size_type max_size() const noexcept { return m_data.max_size(); }

  void clear() { m_data.clear(); }

  std::pair<iterator, bool> insert(const value_type& value) { return insert_impl(value); }

  std::pair<iterator, bool> insert(value_type&& value)
  {
    return insert_impl(std::move(value));
  }

  iterator insert(const_iterator hint, const value_type& value)
  {
    return insert_impl(hint, value).first;
  }

  iterator insert(const_iterator hint, value_type&& value)
  {
    return insert_impl(hint, std::move(value)).first;
  }

  template <typename K>
    requires is_transparent_v<Compare> && std::is_constructible_v<value_type, K&&>
  std::pair<iterator, bool> insert(K&& x)
  {
    auto pos = lower_bound_impl(x);
    if (pos != end() && is_equivalent(x, *pos))
    {
      return {pos, false};
    }
    return {m_data.insert(pos, value_type{std::forward<K>(x)}), true};
  }

  template <typename K>
    requires is_transparent_v<Compare> && std::is_constructible_v<value_type, K&&>
  iterator insert(const_iterator hint, K&& x)
  {
    if (is_valid_hint(hint, x))
    {
      return m_data.insert(hint, value_type{std::forward<K>(x)});
    }
    return insert(std::forward<K>(x)).first;
  }

  template <typename InputIt>
  void insert(InputIt first, InputIt last)
  {
    const auto mid_offset = m_data.size();
    m_data.insert(m_data.end(), first, last);
    merge_unsorted_tail(mid_offset);
  }

  template <typename InputIt>
  void insert(sorted_unique_t, InputIt first, InputIt last)
  {
    const auto mid_offset = m_data.size();
    m_data.insert(m_data.end(), first, last);
    merge_sorted_tail(mid_offset);
  }

  void insert(std::initializer_list<value_type> ilist)
  {
    insert(ilist.begin(), ilist.end());
  }

  void insert(sorted_unique_t, std::initializer_list<value_type> ilist)
  {
    insert(sorted_unique, ilist.begin(), ilist.end());
  }

  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
  void insert_range(R&& range)
  {
    const auto mid_offset = m_data.size();
    for (auto&& value : range)
    {
      m_data.insert(m_data.end(), std::forward<decltype(value)>(value));
    }
    merge_unsorted_tail(mid_offset);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args)
  {
    return insert(value_type{std::forward<Args>(args)...});
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args)
  {
    return insert(hint, value_type{std::forward<Args>(args)...});
  }

  iterator erase(const_iterator pos) { return m_data.erase(pos); }

  iterator erase(const_iterator first, const_iterator last)
  {
    return m_data.erase(first, last);
  }

  size_type erase(const key_type& key)
  {
    const auto [first, last] = equal_range(key);
    const auto count = static_cast<size_type>(std::distance(first, last));
    m_data.erase(first, last);
    return count;
  }

  void swap(flat_set& other) noexcept(
    std::is_nothrow_swappable_v<container_type> && std::is_nothrow_swappable_v<Compare>)
  {
    using std::swap;
    swap(m_data, other.m_data);
    swap(m_cmp, other.m_cmp);
  }

  const_iterator find(const key_type& key) const { return find_impl(key); }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator find(const K& key) const
  {
    return find_impl(key);
  }

  size_type count(const key_type& key) const { return find(key) != end() ? 1u : 0u; }

  template <typename K>
    requires is_transparent_v<Compare>
  size_type count(const K& key) const
  {
    return find(key) != end() ? 1u : 0u;
  }

  bool contains(const key_type& key) const { return find(key) != end(); }

  template <typename K>
    requires is_transparent_v<Compare>
  bool contains(const K& key) const
  {
    return find(key) != end();
  }

  const_iterator lower_bound(const key_type& key) const { return lower_bound_impl(key); }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator lower_bound(const K& key) const
  {
    return lower_bound_impl(key);
  }

  const_iterator upper_bound(const key_type& key) const { return upper_bound_impl(key); }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator upper_bound(const K& key) const
  {
    return upper_bound_impl(key);
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
  {
    return equal_range_impl(key);
  }

  template <typename K>
    requires is_transparent_v<Compare>
  std::pair<const_iterator, const_iterator> equal_range(const K& key) const
  {
    return equal_range_impl(key);
  }

  key_compare key_comp() const { return m_cmp; }

  value_compare value_comp() const { return m_cmp; }

  container_type extract()
  {
    auto result = std::move(m_data);
    m_data.clear();
    return result;
  }

  void replace(container_type&& cont)
  {
    m_data = std::move(cont);
    assert(is_sorted_unique());
  }

  friend bool operator==(const flat_set& lhs, const flat_set& rhs)
  {
    return lhs.m_data == rhs.m_data;
  }

  friend auto operator<=>(const flat_set& lhs, const flat_set& rhs)
  {
    return lhs.m_data <=> rhs.m_data;
  }

private:
  template <typename L, typename R>
  bool is_equivalent(const L& lhs, const R& rhs) const
  {
    return !m_cmp(lhs, rhs) && !m_cmp(rhs, lhs);
  }

  // indicates whether value would be inserted right before hint, i.e. whether the
  // elements surrounding hint are strictly less than and strictly greater than value,
  // respectively; if so, the caller can insert at hint directly without a binary search,
  // and without checking for an equivalent value, since a value strictly between two
  // adjacent elements of a sorted, duplicate-free container cannot already be present
  template <typename K>
  bool is_valid_hint(const_iterator hint, const K& value) const
  {
    if (hint != end() && !m_cmp(value, *hint))
    {
      return false;
    }
    if (hint != begin() && !m_cmp(*std::prev(hint), value))
    {
      return false;
    }
    return true;
  }

  template <typename K>
  const_iterator find_impl(const K& key) const
  {
    auto pos = lower_bound_impl(key);
    return pos != end() && is_equivalent(key, *pos) ? pos : end();
  }

  // a set never contains two equivalent values, so the equal range is either empty or
  // exactly one element; finding it needs only lower_bound plus a single comparator check
  template <typename K>
  std::pair<const_iterator, const_iterator> equal_range_impl(const K& key) const
  {
    const auto pos = lower_bound_impl(key);
    return pos != end() && is_equivalent(key, *pos) ? std::pair{pos, std::next(pos)}
                                                    : std::pair{pos, pos};
  }

  template <typename K>
  const_iterator lower_bound_impl(const K& key) const
  {
    return std::lower_bound(m_data.begin(), m_data.end(), key, m_cmp);
  }

  template <typename K>
  const_iterator upper_bound_impl(const K& key) const
  {
    return std::upper_bound(m_data.begin(), m_data.end(), key, m_cmp);
  }

  template <typename V>
  std::pair<iterator, bool> insert_impl(V&& value)
  {
    auto pos = lower_bound_impl(value);
    if (pos != end() && is_equivalent(value, *pos))
    {
      return {pos, false};
    }
    return {m_data.insert(pos, std::forward<V>(value)), true};
  }

  template <typename V>
  std::pair<iterator, bool> insert_impl(const_iterator hint, V&& value)
  {
    if (is_valid_hint(hint, value))
    {
      return {m_data.insert(hint, std::forward<V>(value)), true};
    }
    return insert_impl(std::forward<V>(value));
  }

  void dedup()
  {
    m_data.erase(
      std::unique(
        m_data.begin(),
        m_data.end(),
        [this](const auto& lhs, const auto& rhs) {
          return !m_cmp(lhs, rhs) && !m_cmp(rhs, lhs);
        }),
      m_data.end());
  }

  void sort_and_dedup()
  {
    std::sort(m_data.begin(), m_data.end(), m_cmp);
    dedup();
  }

  // merges a sorted tail (starting at mid_offset) that was appended to m_data with the
  // already sorted, duplicate-free prefix in front of it, and removes duplicates; since
  // inplace_merge is stable, pre-existing values take precedence over newly inserted,
  // equivalent values
  void merge_sorted_tail(const size_type mid_offset)
  {
    auto mid = std::next(m_data.begin(), static_cast<difference_type>(mid_offset));
    assert(std::is_sorted(mid, m_data.end(), m_cmp));
    std::inplace_merge(m_data.begin(), mid, m_data.end(), m_cmp);
    dedup();
  }

  // sorts a tail that was appended to m_data (starting at mid_offset), then merges it
  // with the already sorted, duplicate-free prefix in front of it and removes duplicates
  void merge_unsorted_tail(const size_type mid_offset)
  {
    auto mid = std::next(m_data.begin(), static_cast<difference_type>(mid_offset));
    std::sort(mid, m_data.end(), m_cmp);
    merge_sorted_tail(mid_offset);
  }

  bool is_sorted_unique() const
  {
    return std::adjacent_find(
             m_data.begin(),
             m_data.end(),
             [this](const auto& lhs, const auto& rhs) { return !m_cmp(lhs, rhs); })
           == m_data.end();
  }
};

template <typename Key, typename Compare, typename KeyContainer>
void swap(
  flat_set<Key, Compare, KeyContainer>& lhs,
  flat_set<Key, Compare, KeyContainer>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

template <typename Key, typename Compare, typename KeyContainer, typename Pred>
typename flat_set<Key, Compare, KeyContainer>::size_type erase_if(
  flat_set<Key, Compare, KeyContainer>& c, Pred pred)
{
  auto data = c.extract();
  const auto original_size = data.size();
  data.erase(
    std::remove_if(
      data.begin(), data.end(), [&pred](const auto& elem) { return bool(pred(elem)); }),
    data.end());
  const auto removed = original_size - data.size();
  c.replace(std::move(data));
  return removed;
}

template <
  typename KeyContainer,
  typename Compare = std::less<typename KeyContainer::value_type>>
flat_set(KeyContainer, Compare = Compare())
  -> flat_set<typename KeyContainer::value_type, Compare, KeyContainer>;

template <
  typename KeyContainer,
  typename Compare = std::less<typename KeyContainer::value_type>>
flat_set(sorted_unique_t, KeyContainer, Compare = Compare())
  -> flat_set<typename KeyContainer::value_type, Compare, KeyContainer>;

template <
  typename InputIt,
  typename Compare = std::less<typename std::iterator_traits<InputIt>::value_type>>
flat_set(InputIt, InputIt, Compare = Compare())
  -> flat_set<typename std::iterator_traits<InputIt>::value_type, Compare>;

template <
  typename InputIt,
  typename Compare = std::less<typename std::iterator_traits<InputIt>::value_type>>
flat_set(sorted_unique_t, InputIt, InputIt, Compare = Compare())
  -> flat_set<typename std::iterator_traits<InputIt>::value_type, Compare>;

template <typename Key, typename Compare = std::less<Key>>
flat_set(std::initializer_list<Key>, Compare = Compare()) -> flat_set<Key, Compare>;

template <typename Key, typename Compare = std::less<Key>>
flat_set(sorted_unique_t, std::initializer_list<Key>, Compare = Compare())
  -> flat_set<Key, Compare>;

} // namespace kdl
