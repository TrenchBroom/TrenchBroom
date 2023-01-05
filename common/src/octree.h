/*
 Copyright (C) 2022 Kristian Duske

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
#include "Exceptions.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/intersection.h>
#include <vecmath/ray.h>
#include <vecmath/scalar.h>

#include <kdl/overload.h>
#include <kdl/reflection_decl.h>
#include <kdl/reflection_impl.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <ostream>
#include <unordered_map>
#include <variant>
#include <vector>

namespace TrenchBroom
{
namespace detail
{

struct node_address
{
  int16_t x;
  int16_t y;
  int16_t z;
  uint16_t size;

  node_address(int16_t i_x, int16_t i_y, int16_t i_z, uint16_t i_size);

  vm::vec<int, 3> min() const;

  vm::vec<int, 3> max() const;

  bool contains(const node_address& other) const;

  template <typename T>
  vm::bbox<T, 3> to_bounds(const T min_size) const
  {
    return {vm::vec<T, 3>{min()} * min_size, vm::vec<T, 3>{max()} * min_size};
  }

  kdl_reflect_decl(node_address, x, y, z, size);
};

template <typename T>
node_address get_address(const vm::vec<T, 3>& point, const T min_size)
{
  return {
    int16_t(std::floor(point.x() / min_size)),
    int16_t(std::floor(point.y() / min_size)),
    int16_t(std::floor(point.z() / min_size)),
    0};
}

node_address get_parent(const node_address& a);

std::optional<size_t> get_quadrant(const node_address& outer, const node_address& inner);

node_address get_child(const node_address& a, size_t quadrant);

bool is_root(const node_address& a);

node_address get_root(const node_address& address);

node_address get_container(const node_address& address1, const node_address& address2);

template <typename T>
node_address get_container(const vm::bbox<T, 3>& bounds, const T min_size)
{
  // Check if any dimension of the bounding box crosses zero.
  const auto sum_of_signs = vm::sign(bounds.min) + vm::sign(bounds.max);
  if (sum_of_signs.x() == T(0) || sum_of_signs.y() == T(0) || sum_of_signs.z() == T(0))
  {
    // The returned address denotes a bounding box that centers around 0,0,0.
    const auto abs_max =
      vm::abs(vm::get_abs_max_component(vm::abs_max(bounds.min, bounds.max)));
    const auto len = T(2) * abs_max;
    const auto n = std::max(T(2), len / min_size);
    const auto size = std::ceil(std::log2(n));
    const auto c = -std::exp2(size - T(1));
    return {int16_t(c), int16_t(c), int16_t(c), uint16_t(size)};
  }

  auto min_address = get_address(bounds.min, min_size);
  while (!min_address.to_bounds(min_size).contains(bounds.max))
  {
    min_address = get_parent(min_address);
  }

  return min_address;
}

} // namespace detail

/**
 * An octree that allows for quick ray intersection queries.
 *
 * @tparam T the floating point type
 * @tparam S the number of dimensions for vector types
 * @tparam U the node data to store in the nodes
 */
template <typename T, typename U>
class octree
{
public:
  struct leaf_node
  {
    detail::node_address address;
    std::vector<U> data;

    leaf_node(const leaf_node&) = delete;
    leaf_node(leaf_node&&) noexcept = default;

    leaf_node& operator=(const leaf_node&) = delete;
    leaf_node& operator=(leaf_node&&) noexcept = default;

    kdl_reflect_inline(leaf_node, address, data);
  };

  struct inner_node;

  using node = std::variant<leaf_node, inner_node>;

  struct inner_node
  {
    detail::node_address address;
    std::vector<U> data;
    std::vector<node> children;

    inner_node(
      const detail::node_address i_address,
      std::vector<U> i_data,
      std::vector<node> i_children)
      : address{i_address}
      , data{std::move(i_data)}
      , children{std::move(i_children)}
    {
    }

    inner_node(const detail::node_address i_address, std::vector<U> i_data)
      : inner_node{
        i_address,
        std::move(i_data),
        kdl::vec_from(
          node{leaf_node{get_child(i_address, 0), {}}},
          node{leaf_node{get_child(i_address, 1), {}}},
          node{leaf_node{get_child(i_address, 2), {}}},
          node{leaf_node{get_child(i_address, 3), {}}},
          node{leaf_node{get_child(i_address, 4), {}}},
          node{leaf_node{get_child(i_address, 5), {}}},
          node{leaf_node{get_child(i_address, 6), {}}},
          node{leaf_node{get_child(i_address, 7), {}}})}
    {
    }

    inner_node(const inner_node&) = delete;
    inner_node(inner_node&&) noexcept = default;

    inner_node& operator=(const inner_node&) = delete;
    inner_node& operator=(inner_node&&) noexcept = default;

    kdl_reflect_inline(inner_node, address, children);
  };

private:
  static detail::node_address& get_address(node& node)
  {
    return std::visit([](auto& x) -> detail::node_address& { return x.address; }, node);
  }

  static const detail::node_address& get_address(const node& node_)
  {
    return const_cast<const detail::node_address&>(get_address(const_cast<node&>(node_)));
  }

  static std::vector<U>& get_data(node& node)
  {
    return std::visit([](auto& x) -> std::vector<U>& { return x.data; }, node);
  }

  static const std::vector<U>& get_data(const node& node_)
  {
    return const_cast<const std::vector<U>&>(get_data(const_cast<node&>(node_)));
  }

  static bool is_inner_node(const node& node)
  {
    return std::visit(
      kdl::overload(
        [](const inner_node&) { return true; }, [](const leaf_node&) { return false; }),
      node);
  }

  static bool is_leaf_node(const node& node) { return !is_inner_node(node); }

  static bool is_empty(const node& node)
  {
    return std::visit(
      kdl::overload(
        [](const inner_node& i) {
          return i.data.empty()
                 && std::all_of(
                   i.children.begin(), i.children.end(), [](const auto& child) {
                     return is_leaf_node(child) && is_empty(child);
                   });
        },
        [](const leaf_node& l) { return l.data.empty(); }),
      node);
  }

  template <typename Predicate, typename Visitor>
  static void visit_node_if(
    const node& node, const Visitor& visitor, const Predicate& predicate)
  {
    if (predicate(node))
    {
      visitor(node);
      std::visit(
        kdl::overload(
          [&](const inner_node& inner_node) {
            for (const auto& child : inner_node.children)
            {
              visit_node_if(child, visitor, predicate);
            }
          },
          [&](const leaf_node&) {}),
        node);
    }
  }

  static void update_root_address(
    node& root,
    const detail::node_address& address,
    std::unordered_map<U, detail::node_address>& node_address_for_data)
  {
    assert(is_root(address));
    assert(address.contains(get_address(root)));
    get_address(root) = address;

    for (const auto& d : get_data(root))
    {
      node_address_for_data.insert_or_assign(d, address);
    }
  }

  static void insert_into_node(node& node, const detail::node_address& address, U data)
  {
    if (!get_address(node).contains(address))
    {
      const auto container_address = get_container(get_address(node), address);
      const auto container_quadrant = get_quadrant(container_address, get_address(node));
      assert(container_quadrant.has_value());

      auto container_node = inner_node{container_address, {}};
      container_node.children[*container_quadrant] = std::move(node);
      node = std::move(container_node);
    }

    assert(get_address(node).contains(address));
    if (const auto quadrant = get_quadrant(get_address(node), address))
    {
      std::visit(
        kdl::overload(
          [&](inner_node& i) {
            insert_into_node(i.children[*quadrant], address, std::move(data));
          },
          [&](leaf_node& l) {
            if (l.data.empty())
            {
              l.address = address;
              l.data.push_back(std::move(data));
            }
            else
            {
              node = inner_node{l.address, std::move(l.data)};
              insert_into_node(node, address, std::move(data));
            }
          }),
        node);
    }
    else
    {
      get_data(node).push_back(std::move(data));
    }
  }

  void remove_from_node(node& node, const detail::node_address& address, const U& data)
  {
    std::visit(
      kdl::overload(
        [&](inner_node& i) {
          if (const auto quadrant = get_quadrant(i.address, address))
          {
            remove_from_node(i.children[*quadrant], address, data);
          }
          else
          {
            const auto i_data = std::find(i.data.begin(), i.data.end(), data);
            assert(i_data != i.data.end());
            i.data.erase(i_data);
          }

          if (!is_root(i.address))
          {
            const auto is_non_empty_child = [](const auto& c) {
              return is_inner_node(c) || !get_data(c).empty();
            };
            const auto num_non_empty_children =
              std::count_if(i.children.begin(), i.children.end(), is_non_empty_child);
            if (num_non_empty_children == 0)
            {
              node = leaf_node{i.address, std::move(i.data)};
            }
            else if (num_non_empty_children == 1 && i.data.empty())
            {
              const auto i_non_empty_child =
                std::find_if(i.children.begin(), i.children.end(), is_non_empty_child);
              assert(i_non_empty_child != i.children.end());

              auto child = std::move(*i_non_empty_child);
              node = std::move(child);
            }
          }
        },
        [&](leaf_node& l) {
          const auto i_data = std::find(l.data.begin(), l.data.end(), data);
          assert(i_data != l.data.end());
          l.data.erase(i_data);
        }),
      node);
  }

private:
  std::optional<node> m_root;
  T m_min_size;
  std::unordered_map<U, detail::node_address> m_node_address_for_data;

public:
  explicit octree(const T min_size)
    : m_min_size{min_size}
  {
  }

  octree(const T min_size, node root)
    : m_root{std::move(root)}
    , m_min_size{min_size}
  {
    auto visitor = kdl::overload(
      [&](auto&& self, const inner_node& node) -> void {
        for (const auto& data : node.data)
        {
          m_node_address_for_data.emplace(data, node.address);
        }
        for (const auto& child : node.children)
        {
          std::visit([&](const auto& c) { self(self, c); }, child);
        }
      },
      [&](auto&&, const leaf_node& node) -> void {
        for (const auto& data : node.data)
        {
          m_node_address_for_data.emplace(data, node.address);
        }
      });

    if (m_root)
    {
      std::visit([&](const auto& node) { visitor(visitor, node); }, *m_root);
    }
  }

  /**
   * Indicates whether a node with the given data exists in this tree.
   *
   * @param data the data to find
   * @return true if a node with the given data exists and false otherwise
   */
  bool contains(const U& data) const { return m_node_address_for_data.count(data) > 0; }

  void insert(const vm::bbox<T, 3>& bounds, U data)
  {
    check(bounds);

    if (contains(data))
    {
      throw NodeTreeException("Data already in tree");
    }

    const auto address = detail::get_container(bounds, m_min_size);
    if (is_root(address))
    {
      if (!m_root)
      {
        m_root = leaf_node{address, {}};
      }
      else if (!get_address(*m_root).contains(address))
      {
        update_root_address(*m_root, address, m_node_address_for_data);
      }

      get_data(*m_root).push_back(std::move(data));
      m_node_address_for_data.emplace(data, get_address(*m_root));
    }
    else
    {
      if (!m_root)
      {
        m_root = inner_node{get_root(address), {}};
      }
      else if (!get_address(*m_root).contains(address))
      {
        update_root_address(*m_root, get_root(address), m_node_address_for_data);
      }

      insert_into_node(*m_root, address, std::move(data));
      m_node_address_for_data.emplace(data, address);
    }
  }


  /**
   * Removes the node with the given data from this tree.
   *
   * @param data the data to remove
   * @return true if a node with the given data was removed, and false otherwise
   */
  bool remove(const U& data)
  {
    const auto i_address = m_node_address_for_data.find(data);
    if (i_address == m_node_address_for_data.end())
    {
      return false;
    }

    remove_from_node(*m_root, i_address->second, data);
    m_node_address_for_data.erase(data);

    if (m_node_address_for_data.empty())
    {
      m_root = std::nullopt;
    }

    return true;
  }

  /**
   * Updates the node with the given data with the given new bounds.
   *
   * @param newBounds the new bounds of the node
   * @param data the node data of the node to update
   *
   * @throws NodeTreeException if no node with the given data can be found in this tree
   */
  void update(const vm::bbox<T, 3>& newBounds, const U& data)
  {
    check(newBounds);

    if (!remove(data))
    {
      throw NodeTreeException("node not found");
    }
    insert(newBounds, data);
  }

  /**
   * Clears this node tree.
   */
  void clear()
  {
    m_node_address_for_data.clear();
    m_root = std::nullopt;
  }

  /**
   * Indicates whether this tree is empty.
   *
   * @return true if this tree is empty and false otherwise
   */
  bool empty() const { return m_root == std::nullopt; }

  /**
   * Finds every data item in this tree whose bounding box intersects with the given ray
   * and retuns a list of those items.
   *
   * @param ray the ray to test
   * @return a list containing all found data items
   */
  std::vector<U> find_intersectors(const vm::ray<T, 3>& ray) const
  {
    auto result = std::vector<U>{};
    find_intersectors(ray, std::back_inserter(result));
    return result;
  }

  /**
   * Finds every data item in this tree whose bounding box intersects with the given ray
   * and appends it to the given output iterator.
   *
   * @tparam O the output iterator type
   * @param ray the ray to test
   * @param out the output iterator to append to
   */
  template <typename O>
  void find_intersectors(const vm::ray<T, 3>& ray, O out) const
  {
    if (m_root)
    {
      visit_node_if(
        *m_root,
        [&](const auto& node) {
          const auto& data = get_data(node);
          std::copy(data.begin(), data.end(), out);
        },
        [&](const auto& node) {
          const auto bounds = get_address(node).to_bounds(m_min_size);
          return bounds.contains(ray.origin)
                 || !vm::is_nan(vm::intersect_ray_bbox(ray, bounds));
        });
    }
  }

  /**
   * Finds every data item in this tree whose bounding box contains the given point and
   * returns a list of those items.
   *
   * @param point the point to test
   * @return a list containing all found data items
   */
  std::vector<U> find_containers(const vm::vec<T, 3>& point) const
  {
    auto result = std::vector<U>{};
    find_containers(point, std::back_inserter(result));
    return result;
  }

  /**
   * Finds every data item in this tree whose bounding box contains the given point and
   * appends it to the given output iterator.
   *
   * @tparam O the output iterator type
   * @param point the point to test
   * @param out the output iterator to append to
   */
  template <typename O>
  void find_containers(const vm::vec<T, 3>& point, O out) const
  {
    if (m_root)
    {
      visit_node_if(
        *m_root,
        [&](const auto& node) {
          const auto& data = get_data(node);
          std::copy(data.begin(), data.end(), out);
        },
        [&](const auto& node) {
          return get_address(node).to_bounds(m_min_size).contains(point);
        });
    }
  }

  kdl_reflect_inline(octree, m_root, m_min_size, m_node_address_for_data);

private:
  void check(const vm::bbox<T, 3>& bounds) const
  {
    if (vm::is_nan(bounds.min) || vm::is_nan(bounds.max))
    {
      throw NodeTreeException("Cannot add node to octree with invalid bounds");
    }
  }
};

} // namespace TrenchBroom
