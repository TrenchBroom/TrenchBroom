/*
 Copyright (C) 2023 Kristian Duske

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

#include <kdl/std_io.h>

#include <algorithm>
#include <sstream>

#include "Catch2.h"

namespace TrenchBroom
{
template <typename T>
class AnyOfMatcher : public Catch::MatcherBase<T>
{
  std::vector<T> m_expected;

public:
  explicit AnyOfMatcher(std::vector<T> expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const T& in) const override
  {
    return std::any_of(
      m_expected.begin(), m_expected.end(), [&](const auto& e) { return in == e; });
  }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << "matches any of " << kdl::make_streamable(m_expected);
    return str.str();
  }
};

template <typename T>
AnyOfMatcher<T> MatchesAnyOf(std::vector<T> expected)
{
  return AnyOfMatcher<T>{std::move(expected)};
}

template <typename T>
AnyOfMatcher<T> MatchesAnyOf(std::initializer_list<T> expected)
{
  return AnyOfMatcher<T>(std::vector<T>{expected});
}

} // namespace TrenchBroom
