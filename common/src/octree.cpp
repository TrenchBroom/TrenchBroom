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

#include "octree.h"

namespace TrenchBroom
{
namespace detail
{
namespace
{
[[maybe_unused]] bool is_valid_address(
  const int16_t x, const int16_t y, const int16_t z, const uint16_t size)
{
  const auto a_size = (1 << size);

  const auto is_root = [&](const auto n) { return n == -a_size / 2; };
  const auto is_valid = [&](const auto n) { return n == (n / a_size) * a_size; };

  return (is_root(x) && is_root(y) && is_root(z))
         || (is_valid(x) && is_valid(y) && is_valid(z));
}
} // namespace

node_address::node_address(
  const int16_t i_x, const int16_t i_y, const int16_t i_z, const uint16_t i_size)
  : x{i_x}
  , y{i_y}
  , z{i_z}
  , size{i_size}
{
  assert(is_valid_address(x, y, z, size));
}

vm::vec<int, 3> node_address::min() const
{
  return {x, y, z};
}

vm::vec<int, 3> node_address::max() const
{
  const auto len = (1 << size);
  return {x + len, y + len, z + len};
}

bool node_address::contains(const node_address& other) const
{
  return vm::bbox{min(), max()}.contains(vm::bbox{other.min(), other.max()});
}

kdl_reflect_impl(node_address);

node_address get_parent(const node_address& a)
{
  const auto p_s = 1 << (a.size + 1);
  return {
    int16_t(((a.x >= 0 ? a.x : a.x - p_s + 1) / p_s) * p_s),
    int16_t(((a.y >= 0 ? a.y : a.y - p_s + 1) / p_s) * p_s),
    int16_t(((a.z >= 0 ? a.z : a.z - p_s + 1) / p_s) * p_s),
    uint16_t(a.size + 1)};
}

std::optional<size_t> get_quadrant(const node_address& outer, const node_address& inner)
{
  assert(outer.contains(inner));

  if (outer.size == 0)
  {
    return std::nullopt;
  }

  const auto half = outer.max() / 2 + outer.min() / 2;
  const auto sum_of_signs = vm::sign(inner.min() - half) + vm::sign(inner.max() - half);
  if (sum_of_signs.x() == 0 || sum_of_signs.y() == 0 || sum_of_signs.z() == 0)
  {
    return std::nullopt;
  }

  return (inner.max().x() <= half.x() ? 0 : 1) | (inner.max().y() <= half.y() ? 0 : 2)
         | (inner.max().z() <= half.z() ? 0 : 4);
}

node_address get_child(const node_address& a, const size_t quadrant)
{
  assert(quadrant < 8);
  assert(a.size > 0);

  return {
    int16_t(a.x + ((quadrant & 1) ? (1 << (a.size - 1)) : 0)),
    int16_t(a.y + ((quadrant & 2) ? (1 << (a.size - 1)) : 0)),
    int16_t(a.z + ((quadrant & 4) ? (1 << (a.size - 1)) : 0)),
    uint16_t(a.size - 1)};
}

bool is_root(const node_address& a)
{
  return vm::abs(a.min()) == vm::abs(a.max());
}

node_address get_root(const node_address& address)
{
  const auto parent = get_parent(address);
  const auto min = parent.min();
  const auto max = parent.max();

  const auto w2 = vm::abs(vm::get_abs_max_component(vm::abs_max(min, max)));
  const auto s = std::ceil(std::log2(2 * w2));
  const auto l2 = std::exp2(s - 1);

  return {int16_t(-l2), int16_t(-l2), int16_t(-l2), uint16_t(s)};
}

node_address get_container(const node_address& address1, const node_address& address2)
{
  assert((address1.x >= 0) == (address2.x >= 0));
  assert((address1.y >= 0) == (address2.y >= 0));
  assert((address1.z >= 0) == (address2.z >= 0));

  if (address1.contains(address2))
  {
    return address1;
  }
  if (address2.contains(address1))
  {
    return address2;
  }

  auto container = get_parent(address1);
  while (!container.contains(address2))
  {
    container = get_parent(container);
  }
  return container;
}
} // namespace detail
} // namespace TrenchBroom
