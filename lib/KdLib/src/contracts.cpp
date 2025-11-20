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

#include "kd/contracts.h"

#include <cstdlib>

namespace kd
{
namespace
{

contract_violation_handler global_violation_handler =
  [](const std::string_view, const int, const std::string_view, const std::string_view) {
    std::abort();
  };

} // namespace


namespace detail
{

[[noreturn]] void contract_violated(
  const std::string_view file,
  const int line,
  const std::string_view type,
  const std::string_view condition)
{
  global_violation_handler(file, line, type, condition);

  // the global violation handler must not return, but to ensure this, we abort here
  std::abort();
}

} // namespace detail

void set_contract_violation_handler(contract_violation_handler violation_handler)
{
  global_violation_handler = std::move(violation_handler);
}

} // namespace kd
