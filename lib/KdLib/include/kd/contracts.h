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

#include <functional>
#include <string_view>

namespace kd
{
namespace detail
{
[[noreturn]] void contract_violated(
  std::string_view file, int line, std::string_view type, std::string_view condition);
}

/**
 * The signature of a contract violation handler
 *
 * It gets called with the following parameters:
 * * the path to the file where the violation occurred
 * * the line number where the violation occurred
 * * the type of the condition that was violated (precondition, postcondition, assertion)
 * * the stringified condition that was violated
 *
 * A contract violation handler must never return!
 */
using contract_violation_handler =
  std::function<void(std::string_view, int, std::string_view, std::string_view)>;

/**
 * Set a violation handler that is called when a contract is violated. The function must
 * not return!
 */
void set_contract_violation_handler(contract_violation_handler violation_handler);

} // namespace kd

// These are ugly but necessary to stringify an expression, see:
// https://en.wikipedia.org/wiki/C_preprocessor#Token_stringification
#define stringification(expression) #expression
#define stringification2(expression) stringification(expression)

#define contract_pre(condition)                                                          \
  do                                                                                     \
  {                                                                                      \
    if (!(condition))                                                                    \
    {                                                                                    \
      kd::detail::contract_violated(                                                     \
        __FILE__, __LINE__, "precondition", stringification2(condition));                \
    }                                                                                    \
  } while (false)

#define contract_post(condition)                                                         \
  do                                                                                     \
  {                                                                                      \
    if (!(condition))                                                                    \
    {                                                                                    \
      kd::detail::contract_violated(                                                     \
        __FILE__, __LINE__, "postcondition", stringification2(condition));               \
    }                                                                                    \
  } while (false)

#define contract_assert(condition)                                                       \
  do                                                                                     \
  {                                                                                      \
    if (!(condition))                                                                    \
    {                                                                                    \
      kd::detail::contract_violated(                                                     \
        __FILE__, __LINE__, "assertion", stringification2(condition));                   \
    }                                                                                    \
  } while (false)
