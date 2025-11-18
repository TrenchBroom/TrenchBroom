/*
 Copyright (C) 2025 Kristian Duske

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

#include <utility>

namespace upd
{

/**
 * Creates a type that inherits from all of its type parameters and `operator()` from each
 * supertype. Can be used with std::visit to create on-the-fly visitors from lambdas. So
 * usually, the supertypes are lambdas.
 *
 * @see https://dev.to/tmr232/that-overloaded-trick-overloading-lambdas-in-c17
 *
 * @tparam Ts the lambdas to inherit from
 */
template <typename... Ts>
struct overload_impl : Ts...
{
  using Ts::operator()...;
  explicit overload_impl(Ts&&... ts)
    : Ts{std::forward<Ts>(ts)}...
  {
  }
};

/**
 * Factory function to create an overload.
 */
template <typename... Ts>
auto overload(Ts&&... ts)
{
  return overload_impl<Ts...>{std::forward<Ts>(ts)...};
}

} // namespace upd
