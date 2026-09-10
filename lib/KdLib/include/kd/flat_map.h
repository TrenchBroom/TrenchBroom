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

#include "kd/flat_map_forward.h"
#include "kd/pair_common_reference.h" // IWYU pragma: keep
#include "kd/sorted_unique.h"
#include "kd/traits.h"

#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace kdl
{

// A backport of std::flat_map (C++23). Keys and values are stored in two separate
// parallel containers (by default std::vector) rather than as an array of pairs, so
// dereferencing an iterator synthesizes a pair-like proxy of references rather than
// yielding a reference to an actually stored std::pair.
//
// default template arguments are defined in flat_map_forward.h
template <
  typename Key,
  typename T,
  typename Compare,
  typename KeyContainer,
  typename MappedContainer>
class flat_map
{
private:
  template <bool Const>
  class iterator_impl
  {
  private:
    using key_iterator = typename KeyContainer::const_iterator;
    using mapped_iterator = std::conditional_t<
      Const,
      typename MappedContainer::const_iterator,
      typename MappedContainer::iterator>;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::pair<Key, T>;
    using difference_type = typename KeyContainer::difference_type;
    using reference = std::pair<const Key&, std::conditional_t<Const, const T&, T&>>;

    // proxy returned by operator->() so that it->second works despite operator*()
    // returning a proxy reference rather than a real reference
    struct pointer
    {
      reference ref;
      const reference* operator->() const noexcept { return &ref; }
    };

    iterator_impl() = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr iterator_impl(const iterator_impl<!Const>& other)
      requires Const
      : m_key_it{other.m_key_it}
      , m_mapped_it{other.m_mapped_it}
    {
    }

    constexpr reference operator*() const { return reference{*m_key_it, *m_mapped_it}; }
    constexpr pointer operator->() const { return pointer{**this}; }
    constexpr reference operator[](const difference_type n) const { return *(*this + n); }

    constexpr iterator_impl& operator++()
    {
      ++m_key_it;
      ++m_mapped_it;
      return *this;
    }

    constexpr iterator_impl operator++(int)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator_impl& operator--()
    {
      --m_key_it;
      --m_mapped_it;
      return *this;
    }

    constexpr iterator_impl operator--(int)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator_impl& operator+=(const difference_type n)
    {
      m_key_it += n;
      m_mapped_it += n;
      return *this;
    }

    constexpr iterator_impl& operator-=(const difference_type n)
    {
      m_key_it -= n;
      m_mapped_it -= n;
      return *this;
    }

    friend constexpr iterator_impl operator+(iterator_impl it, const difference_type n)
    {
      it += n;
      return it;
    }

    friend constexpr iterator_impl operator+(const difference_type n, iterator_impl it)
    {
      it += n;
      return it;
    }

    friend constexpr iterator_impl operator-(iterator_impl it, const difference_type n)
    {
      it -= n;
      return it;
    }

    friend constexpr difference_type operator-(
      const iterator_impl& lhs, const iterator_impl& rhs)
    {
      return lhs.m_key_it - rhs.m_key_it;
    }

    friend constexpr bool operator==(const iterator_impl& lhs, const iterator_impl& rhs)
    {
      return lhs.m_key_it == rhs.m_key_it;
    }

    friend constexpr auto operator<=>(const iterator_impl& lhs, const iterator_impl& rhs)
    {
      return lhs.m_key_it <=> rhs.m_key_it;
    }

  private:
    friend class flat_map;
    template <bool>
    friend class iterator_impl;

    constexpr iterator_impl(key_iterator key_it, mapped_iterator mapped_it)
      : m_key_it{key_it}
      , m_mapped_it{mapped_it}
    {
    }

    key_iterator m_key_it{};
    mapped_iterator m_mapped_it{};
  };

public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;
  using key_compare = Compare;
  using key_container_type = KeyContainer;
  using mapped_container_type = MappedContainer;
  using reference = std::pair<const key_type&, mapped_type&>;
  using const_reference = std::pair<const key_type&, const mapped_type&>;
  using size_type = typename KeyContainer::size_type;
  using difference_type = typename KeyContainer::difference_type;
  using iterator = iterator_impl<false>;
  using const_iterator = iterator_impl<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  struct containers
  {
    key_container_type keys;
    mapped_container_type values;
  };

  class value_compare
  {
  protected:
    Compare comp;

    explicit value_compare(Compare c)
      : comp{std::move(c)}
    {
    }

    friend class flat_map;

  public:
    bool operator()(const value_type& lhs, const value_type& rhs) const
    {
      return comp(lhs.first, rhs.first);
    }
  };

  static_assert(
    std::is_same_v<Key, typename KeyContainer::value_type>,
    "flat_map's Key must be the same type as KeyContainer::value_type");
  static_assert(
    std::is_same_v<T, typename MappedContainer::value_type>,
    "flat_map's T must be the same type as MappedContainer::value_type");

private:
  KeyContainer m_keys;
  MappedContainer m_values;
  Compare m_cmp;

public:
  flat_map() = default;

  explicit flat_map(const key_compare& comp)
    : m_cmp{comp}
  {
  }

  explicit flat_map(containers ctnrs, const key_compare& comp = key_compare())
    : m_keys{std::move(ctnrs.keys)}
    , m_values{std::move(ctnrs.values)}
    , m_cmp{comp}
  {
    sort_and_dedup();
  }

  flat_map(sorted_unique_t, containers ctnrs, const key_compare& comp = key_compare())
    : m_keys{std::move(ctnrs.keys)}
    , m_values{std::move(ctnrs.values)}
    , m_cmp{comp}
  {
    assert(is_sorted_unique());
  }

  template <typename InputIt>
  flat_map(InputIt first, InputIt last, const key_compare& comp = key_compare())
    : m_cmp{comp}
  {
    append(first, last);
    sort_and_dedup();
  }

  template <typename InputIt>
  flat_map(
    sorted_unique_t, InputIt first, InputIt last, const key_compare& comp = key_compare())
    : m_cmp{comp}
  {
    append(first, last);
    assert(is_sorted_unique());
  }

  flat_map(
    std::initializer_list<value_type> init, const key_compare& comp = key_compare())
    : flat_map{init.begin(), init.end(), comp}
  {
  }

  flat_map(
    sorted_unique_t,
    std::initializer_list<value_type> init,
    const key_compare& comp = key_compare())
    : flat_map{sorted_unique, init.begin(), init.end(), comp}
  {
  }

  flat_map& operator=(std::initializer_list<value_type> init)
  {
    m_keys.clear();
    m_values.clear();
    for (const auto& v : init)
    {
      m_keys.push_back(v.first);
      m_values.push_back(v.second);
    }
    sort_and_dedup();
    return *this;
  }

  iterator begin() noexcept { return iterator{m_keys.begin(), m_values.begin()}; }

  const_iterator begin() const noexcept
  {
    return const_iterator{m_keys.begin(), m_values.begin()};
  }

  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept { return iterator{m_keys.end(), m_values.end()}; }

  const_iterator end() const noexcept
  {
    return const_iterator{m_keys.end(), m_values.end()};
  }

  const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }

  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }

  const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }

  const_reverse_iterator crend() const noexcept { return rend(); }

  bool empty() const noexcept { return m_keys.empty(); }

  size_type size() const noexcept { return m_keys.size(); }

  size_type max_size() const noexcept
  {
    return std::min(m_keys.max_size(), m_values.max_size());
  }

  void clear()
  {
    m_keys.clear();
    m_values.clear();
  }

  std::pair<iterator, bool> insert(const value_type& value)
  {
    return emplace_at(value.first, value.second);
  }

  std::pair<iterator, bool> insert(value_type&& value)
  {
    return emplace_at(std::move(value.first), std::move(value.second));
  }

  template <typename P>
    requires std::is_constructible_v<value_type, P&&>
  std::pair<iterator, bool> insert(P&& p)
  {
    auto v = value_type{std::forward<P>(p)};
    return emplace_at(std::move(v.first), std::move(v.second));
  }

  iterator insert(const_iterator hint, const value_type& value)
  {
    return emplace_at(hint, value.first, value.second).first;
  }

  iterator insert(const_iterator hint, value_type&& value)
  {
    return emplace_at(hint, std::move(value.first), std::move(value.second)).first;
  }

  template <typename P>
    requires std::is_constructible_v<value_type, P&&>
  iterator insert(const_iterator hint, P&& p)
  {
    auto v = value_type{std::forward<P>(p)};
    return emplace_at(hint, std::move(v.first), std::move(v.second)).first;
  }

  template <typename InputIt>
  void insert(InputIt first, InputIt last)
  {
    const auto mid_offset = m_keys.size();
    append(first, last);
    merge_unsorted_tail(mid_offset);
  }

  template <typename InputIt>
  void insert(sorted_unique_t, InputIt first, InputIt last)
  {
    const auto mid_offset = m_keys.size();
    append(first, last);
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
    const auto mid_offset = m_keys.size();
    for (auto&& elem : range)
    {
      auto v = value_type{std::forward<decltype(elem)>(elem)};
      m_keys.push_back(std::move(v.first));
      m_values.push_back(std::move(v.second));
    }
    merge_unsorted_tail(mid_offset);
  }

  template <typename M>
  std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& value)
  {
    return insert_or_assign_impl(key, std::forward<M>(value));
  }

  template <typename M>
  std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& value)
  {
    return insert_or_assign_impl(std::move(key), std::forward<M>(value));
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args)
  {
    return emplace_at(key, std::forward<Args>(args)...);
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args)
  {
    return emplace_at(std::move(key), std::forward<Args>(args)...);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args)
  {
    auto v = value_type{std::forward<Args>(args)...};
    return emplace_at(std::move(v.first), std::move(v.second));
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args)
  {
    auto v = value_type{std::forward<Args>(args)...};
    return emplace_at(hint, std::move(v.first), std::move(v.second)).first;
  }

  iterator erase(const_iterator pos)
  {
    const auto idx = index_of(pos);
    erase_at(idx, idx + 1u);
    return make_iterator(idx);
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    const auto first_idx = index_of(first);
    const auto last_idx = index_of(last);
    erase_at(first_idx, last_idx);
    return make_iterator(first_idx);
  }

  size_type erase(const key_type& key)
  {
    const auto idx = find_index(key);
    if (idx == m_keys.size())
    {
      return 0u;
    }
    erase_at(idx, idx + 1u);
    return 1u;
  }

  void swap(flat_map& other) noexcept(
    std::is_nothrow_swappable_v<key_container_type>
    && std::is_nothrow_swappable_v<mapped_container_type>
    && std::is_nothrow_swappable_v<Compare>)
  {
    using std::swap;
    swap(m_keys, other.m_keys);
    swap(m_values, other.m_values);
    swap(m_cmp, other.m_cmp);
  }

  mapped_type& operator[](const key_type& key) { return try_emplace(key).first->second; }

  mapped_type& operator[](key_type&& key)
  {
    return try_emplace(std::move(key)).first->second;
  }

  mapped_type& at(const key_type& key)
  {
    const auto idx = find_index(key);
    if (idx == m_keys.size())
    {
      throw std::out_of_range{"kdl::flat_map::at"};
    }
    return m_values[idx];
  }

  const mapped_type& at(const key_type& key) const
  {
    const auto idx = find_index(key);
    if (idx == m_keys.size())
    {
      throw std::out_of_range{"kdl::flat_map::at"};
    }
    return m_values[idx];
  }

  iterator find(const key_type& key) { return make_iterator(find_index(key)); }

  const_iterator find(const key_type& key) const
  {
    return make_iterator(find_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  iterator find(const K& key)
  {
    return make_iterator(find_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator find(const K& key) const
  {
    return make_iterator(find_index(key));
  }

  size_type count(const key_type& key) const { return contains(key) ? 1u : 0u; }

  template <typename K>
    requires is_transparent_v<Compare>
  size_type count(const K& key) const
  {
    return contains(key) ? 1u : 0u;
  }

  bool contains(const key_type& key) const { return find_index(key) != m_keys.size(); }

  template <typename K>
    requires is_transparent_v<Compare>
  bool contains(const K& key) const
  {
    return find_index(key) != m_keys.size();
  }

  iterator lower_bound(const key_type& key)
  {
    return make_iterator(lower_bound_index(key));
  }

  const_iterator lower_bound(const key_type& key) const
  {
    return make_iterator(lower_bound_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  iterator lower_bound(const K& key)
  {
    return make_iterator(lower_bound_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator lower_bound(const K& key) const
  {
    return make_iterator(lower_bound_index(key));
  }

  iterator upper_bound(const key_type& key)
  {
    return make_iterator(upper_bound_index(key));
  }

  const_iterator upper_bound(const key_type& key) const
  {
    return make_iterator(upper_bound_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  iterator upper_bound(const K& key)
  {
    return make_iterator(upper_bound_index(key));
  }

  template <typename K>
    requires is_transparent_v<Compare>
  const_iterator upper_bound(const K& key) const
  {
    return make_iterator(upper_bound_index(key));
  }

  std::pair<iterator, iterator> equal_range(const key_type& key)
  {
    const auto [lo, hi] = equal_range_index(key);
    return {make_iterator(lo), make_iterator(hi)};
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
  {
    const auto [lo, hi] = equal_range_index(key);
    return {make_iterator(lo), make_iterator(hi)};
  }

  template <typename K>
    requires is_transparent_v<Compare>
  std::pair<iterator, iterator> equal_range(const K& key)
  {
    const auto [lo, hi] = equal_range_index(key);
    return {make_iterator(lo), make_iterator(hi)};
  }

  template <typename K>
    requires is_transparent_v<Compare>
  std::pair<const_iterator, const_iterator> equal_range(const K& key) const
  {
    const auto [lo, hi] = equal_range_index(key);
    return {make_iterator(lo), make_iterator(hi)};
  }

  key_compare key_comp() const { return m_cmp; }

  value_compare value_comp() const { return value_compare{m_cmp}; }

  containers extract()
  {
    auto result = containers{std::move(m_keys), std::move(m_values)};
    m_keys.clear();
    m_values.clear();
    return result;
  }

  void replace(key_container_type&& keys, mapped_container_type&& values)
  {
    m_keys = std::move(keys);
    m_values = std::move(values);
    assert(is_sorted_unique());
  }

  // This accessor (and values() below) exists to work around a gap in this backport, not
  // a limitation of flat_map itself: dereferencing this map's iterator synthesizes a
  // `pair<const Key&, T&>` by value rather than yielding a reference to an actually
  // stored pair, since keys and values live in separate parallel containers. For that
  // proxy pair to satisfy `std::indirectly_readable` (and hence `std::input_iterator`,
  // and hence work with C++20 ranges algorithms and views such as `std::ranges::all_of`
  // or `std::views::keys` / `std::views::values`), the standard library needs the
  // `std::pair`/`std::tuple` `common_reference` support that C++23 added via P2165. A
  // conforming `std::flat_map` gets that for free from its standard library.
  //
  // kd/pair_common_reference.h backports just enough of P2165 to make this work through a
  // `const_iterator`, which covers the common case of iterating or ranges-piping a map
  // passed by `const&` (e.g. `constMap | std::views::keys` compiles fine as long as
  // `pair_common_reference.h` has been included). It is NOT enough to make it work
  // through the mutable `iterator`: `std::pair`'s own converting constructor also needs a
  // C++23-only overload taking `pair<U1, U2>&` (not just `const pair<U1, U2>&`) to
  // construct a proxy pair with a non-const reference member, and that constructor lives
  // on `std::pair` itself, which a program cannot patch by specializing a trait. So
  // `mutableMap | std::views::values` (or any ranges algorithm/view applied to a
  // non-const flat_map or its mutable iterator) still fails to compile under C++20.
  //
  // keys() and values() sidestep that remaining gap by handing back the real, non-proxy
  // underlying containers directly. Being ordinary vectors, they satisfy every range
  // concept trivially regardless of constness, so `m.keys() | std::views::...` works
  // where `m | std::views::keys` would not (for a mutable m) or would require
  // pair_common_reference.h to be included (for a const m).
  //
  // If this codebase upgrades to C++23 and replaces kdl::flat_map with std::flat_map (on
  // a standard library that implements P2165), these accessors are no longer needed:
  // replace `m.keys()` with `m | std::views::keys`, `m.values()` with
  // `m | std::views::values`, and delete keys()/values() from kdl::flat_map.
  const key_container_type& keys() const noexcept { return m_keys; }

  const mapped_container_type& values() const noexcept { return m_values; }

  friend bool operator==(const flat_map& lhs, const flat_map& rhs)
  {
    return lhs.m_keys == rhs.m_keys && lhs.m_values == rhs.m_values;
  }

private:
  template <typename L, typename R>
  bool is_equivalent(const L& lhs, const R& rhs) const
  {
    return !m_cmp(lhs, rhs) && !m_cmp(rhs, lhs);
  }

  template <typename K>
  size_type lower_bound_index(const K& key) const
  {
    return static_cast<size_type>(
      std::lower_bound(m_keys.begin(), m_keys.end(), key, m_cmp) - m_keys.begin());
  }

  template <typename K>
  size_type upper_bound_index(const K& key) const
  {
    return static_cast<size_type>(
      std::upper_bound(m_keys.begin(), m_keys.end(), key, m_cmp) - m_keys.begin());
  }

  template <typename K>
  size_type find_index(const K& key) const
  {
    const auto idx = lower_bound_index(key);
    return idx != m_keys.size() && is_equivalent(key, m_keys[idx]) ? idx : m_keys.size();
  }

  // a map never contains two pairs with equivalent keys, so the equal range is either
  // empty or exactly one pair; finding it needs only lower_bound_index plus a single
  // comparator check, not a second binary search via upper_bound_index (and
  // std::equal_range wouldn't help either, since it can't exploit that invariant and
  // still performs the equivalent of two binary searches for possibly-duplicate ranges)
  template <typename K>
  std::pair<size_type, size_type> equal_range_index(const K& key) const
  {
    const auto idx = lower_bound_index(key);
    return idx != m_keys.size() && is_equivalent(key, m_keys[idx])
             ? std::pair{idx, idx + 1u}
             : std::pair{idx, idx};
  }

  // indicates whether a pair with the given key would be inserted right before idx, i.e.
  // whether the keys surrounding idx are strictly less than and strictly greater than
  // key, respectively; if so, the caller can insert at idx directly without a binary
  // search
  template <typename K>
  bool is_valid_hint(size_type idx, const K& key) const
  {
    if (idx < m_keys.size() && !m_cmp(key, m_keys[idx]))
    {
      return false;
    }
    if (idx > 0 && !m_cmp(m_keys[idx - 1u], key))
    {
      return false;
    }
    return true;
  }

  size_type index_of(const_iterator it) const
  {
    return static_cast<size_type>(it.m_key_it - m_keys.begin());
  }

  iterator make_iterator(const size_type idx)
  {
    return iterator{
      std::next(m_keys.begin(), static_cast<difference_type>(idx)),
      std::next(m_values.begin(), static_cast<difference_type>(idx))};
  }

  const_iterator make_iterator(const size_type idx) const
  {
    return const_iterator{
      std::next(m_keys.begin(), static_cast<difference_type>(idx)),
      std::next(m_values.begin(), static_cast<difference_type>(idx))};
  }

  template <typename InputIt>
  void append(InputIt first, InputIt last)
  {
    for (; first != last; ++first)
    {
      auto v = value_type{*first};
      m_keys.push_back(std::move(v.first));
      m_values.push_back(std::move(v.second));
    }
  }

  void erase_at(const size_type first_idx, const size_type last_idx)
  {
    m_keys.erase(
      std::next(m_keys.begin(), static_cast<difference_type>(first_idx)),
      std::next(m_keys.begin(), static_cast<difference_type>(last_idx)));
    m_values.erase(
      std::next(m_values.begin(), static_cast<difference_type>(first_idx)),
      std::next(m_values.begin(), static_cast<difference_type>(last_idx)));
  }

  template <typename K, typename... Args>
  std::pair<iterator, bool> emplace_at(K&& key, Args&&... args)
  {
    const auto idx = lower_bound_index(key);
    if (idx != m_keys.size() && is_equivalent(key, m_keys[idx]))
    {
      return {make_iterator(idx), false};
    }
    m_keys.insert(
      std::next(m_keys.begin(), static_cast<difference_type>(idx)), std::forward<K>(key));
    m_values.insert(
      std::next(m_values.begin(), static_cast<difference_type>(idx)),
      mapped_type(std::forward<Args>(args)...));
    return {make_iterator(idx), true};
  }

  template <typename K, typename... Args>
  std::pair<iterator, bool> emplace_at(const_iterator hint, K&& key, Args&&... args)
  {
    const auto idx = index_of(hint);
    if (is_valid_hint(idx, key))
    {
      m_keys.insert(
        std::next(m_keys.begin(), static_cast<difference_type>(idx)),
        std::forward<K>(key));
      m_values.insert(
        std::next(m_values.begin(), static_cast<difference_type>(idx)),
        mapped_type(std::forward<Args>(args)...));
      return {make_iterator(idx), true};
    }
    return emplace_at(std::forward<K>(key), std::forward<Args>(args)...);
  }

  template <typename K, typename M>
  std::pair<iterator, bool> insert_or_assign_impl(K&& key, M&& value)
  {
    const auto idx = lower_bound_index(key);
    if (idx != m_keys.size() && is_equivalent(key, m_keys[idx]))
    {
      m_values[idx] = std::forward<M>(value);
      return {make_iterator(idx), false};
    }
    m_keys.insert(
      std::next(m_keys.begin(), static_cast<difference_type>(idx)), std::forward<K>(key));
    m_values.insert(
      std::next(m_values.begin(), static_cast<difference_type>(idx)),
      std::forward<M>(value));
    return {make_iterator(idx), true};
  }

  // moves the (key, value) pairs out of the parallel containers into a single vector of
  // owning pairs for sorting / merging, leaving the parallel containers empty
  std::vector<value_type> extract_entries()
  {
    std::vector<value_type> entries;
    entries.reserve(m_keys.size());
    for (size_type i = 0; i < m_keys.size(); ++i)
    {
      entries.emplace_back(std::move(m_keys[i]), std::move(m_values[i]));
    }
    m_keys.clear();
    m_values.clear();
    return entries;
  }

  // moves the given (key, value) pairs back into the (empty) parallel containers
  void rebuild_from(std::vector<value_type> entries)
  {
    m_keys.reserve(entries.size());
    m_values.reserve(entries.size());
    for (auto& e : entries)
    {
      m_keys.push_back(std::move(e.first));
      m_values.push_back(std::move(e.second));
    }
  }

  void dedup_entries(std::vector<value_type>& entries)
  {
    entries.erase(
      std::unique(
        entries.begin(),
        entries.end(),
        [this](const auto& lhs, const auto& rhs) {
          return !m_cmp(lhs.first, rhs.first) && !m_cmp(rhs.first, lhs.first);
        }),
      entries.end());
  }

  void sort_and_dedup()
  {
    auto entries = extract_entries();
    std::stable_sort(
      entries.begin(), entries.end(), [this](const auto& lhs, const auto& rhs) {
        return m_cmp(lhs.first, rhs.first);
      });
    dedup_entries(entries);
    rebuild_from(std::move(entries));
  }

  // merges a sorted tail (starting at mid_offset) that was appended to the parallel
  // containers with the already sorted, duplicate-free prefix in front of it, and removes
  // duplicates; since inplace_merge is stable, pre-existing pairs take precedence over
  // newly inserted pairs with an equivalent key
  void merge_sorted_tail(size_type mid_offset)
  {
    auto entries = extract_entries();
    auto mid = std::next(entries.begin(), static_cast<difference_type>(mid_offset));
    assert(std::is_sorted(mid, entries.end(), [this](const auto& lhs, const auto& rhs) {
      return m_cmp(lhs.first, rhs.first);
    }));
    std::inplace_merge(
      entries.begin(), mid, entries.end(), [this](const auto& lhs, const auto& rhs) {
        return m_cmp(lhs.first, rhs.first);
      });
    dedup_entries(entries);
    rebuild_from(std::move(entries));
  }

  // sorts a tail that was appended to the parallel containers (starting at mid_offset),
  // then merges it with the already sorted, duplicate-free prefix in front of it and
  // removes duplicates
  void merge_unsorted_tail(size_type mid_offset)
  {
    auto entries = extract_entries();
    auto mid = std::next(entries.begin(), static_cast<difference_type>(mid_offset));
    std::stable_sort(mid, entries.end(), [this](const auto& lhs, const auto& rhs) {
      return m_cmp(lhs.first, rhs.first);
    });
    std::inplace_merge(
      entries.begin(), mid, entries.end(), [this](const auto& lhs, const auto& rhs) {
        return m_cmp(lhs.first, rhs.first);
      });
    dedup_entries(entries);
    rebuild_from(std::move(entries));
  }

  bool is_sorted_unique() const
  {
    for (size_type i = 1; i < m_keys.size(); ++i)
    {
      if (!m_cmp(m_keys[i - 1u], m_keys[i]))
      {
        return false;
      }
    }
    return true;
  }
};

// This is a free function template, not a hidden friend, specifically so that its
// decltype-based return type is only evaluated when it is actually instantiated for a
// call, rather than eagerly whenever flat_map itself is instantiated. A hidden friend's
// signature must be fully resolved as soon as the enclosing flat_map is instantiated
// (since it isn't itself a template), which would make flat_map impossible to use at all
// with any mapped_type lacking operator<=>, even if <=> on
// the map is never actually called.
template <
  typename Key,
  typename T,
  typename Compare,
  typename KeyContainer,
  typename MappedContainer>
std::common_comparison_category_t<
  decltype(std::declval<const Key&>() <=> std::declval<const Key&>()),
  decltype(std::declval<const T&>() <=> std::declval<const T&>())>
operator<=>(
  const flat_map<Key, T, Compare, KeyContainer, MappedContainer>& lhs,
  const flat_map<Key, T, Compare, KeyContainer, MappedContainer>& rhs)
{
  using flat_map_type = flat_map<Key, T, Compare, KeyContainer, MappedContainer>;

  const auto n = std::min(lhs.keys().size(), rhs.keys().size());
  for (typename flat_map_type::size_type i = 0; i < n; ++i)
  {
    if (const auto c = lhs.keys()[i] <=> rhs.keys()[i]; c != 0)
    {
      return c;
    }
    if (const auto c = lhs.values()[i] <=> rhs.values()[i]; c != 0)
    {
      return c;
    }
  }
  return lhs.keys().size() <=> rhs.keys().size();
}

template <
  typename Key,
  typename T,
  typename Compare,
  typename KeyContainer,
  typename MappedContainer>
void swap(
  flat_map<Key, T, Compare, KeyContainer, MappedContainer>& lhs,
  flat_map<Key, T, Compare, KeyContainer, MappedContainer>&
    rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

template <
  typename Key,
  typename T,
  typename Compare,
  typename KeyContainer,
  typename MappedContainer,
  typename Pred>
typename flat_map<Key, T, Compare, KeyContainer, MappedContainer>::size_type erase_if(
  flat_map<Key, T, Compare, KeyContainer, MappedContainer>& c, Pred pred)
{
  using flat_map_type = flat_map<Key, T, Compare, KeyContainer, MappedContainer>;

  auto [keys, values] = c.extract();
  auto entries = std::vector<typename flat_map_type::value_type>{};
  entries.reserve(keys.size());
  for (typename flat_map_type::size_type i = 0; i < keys.size(); ++i)
  {
    entries.emplace_back(std::move(keys[i]), std::move(values[i]));
  }

  const auto original_size = entries.size();
  entries.erase(
    std::remove_if(
      entries.begin(),
      entries.end(),
      [&pred](const auto& e) { return bool(pred(std::as_const(e))); }),
    entries.end());
  const auto removed = original_size - entries.size();

  keys.clear();
  values.clear();
  for (auto& e : entries)
  {
    keys.push_back(std::move(e.first));
    values.push_back(std::move(e.second));
  }
  c.replace(std::move(keys), std::move(values));
  return removed;
}

template <
  typename InputIt,
  typename Compare =
    std::less<typename std::iterator_traits<InputIt>::value_type::first_type>>
flat_map(InputIt, InputIt, Compare = Compare())
  -> flat_map<
    typename std::iterator_traits<InputIt>::value_type::first_type,
    typename std::iterator_traits<InputIt>::value_type::second_type,
    Compare>;

template <
  typename InputIt,
  typename Compare =
    std::less<typename std::iterator_traits<InputIt>::value_type::first_type>>
flat_map(sorted_unique_t, InputIt, InputIt, Compare = Compare())
  -> flat_map<
    typename std::iterator_traits<InputIt>::value_type::first_type,
    typename std::iterator_traits<InputIt>::value_type::second_type,
    Compare>;

template <typename Key, typename T, typename Compare = std::less<Key>>
flat_map(std::initializer_list<std::pair<Key, T>>, Compare = Compare())
  -> flat_map<Key, T, Compare>;

template <typename Key, typename T, typename Compare = std::less<Key>>
flat_map(sorted_unique_t, std::initializer_list<std::pair<Key, T>>, Compare = Compare())
  -> flat_map<Key, T, Compare>;

} // namespace kdl
