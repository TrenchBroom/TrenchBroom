/*
Copyright (C) 2023 Eric Wasylishen

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

#include "Matchers.h"
#include "Result.h"

#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

namespace tb
{

TEST_CASE("ResultMatcher")
{
  SECTION("with matcher")
  {
    using R = Result<std::vector<std::string>>;
    const auto actual = std::vector<std::string>{"asdf", "fdsa"};
    const auto expected = std::vector<std::string>{"fdsa", "asdf"};

    CHECK_THAT(R{actual}, MatchesResult<R>(Catch::Matchers::UnorderedEquals(expected)));
  }

  SECTION("with predicate")
  {
    using R = Result<std::string>;

    CHECK_THAT(
      R{"asdf"}, MatchesResult<R>([](const auto& value) { return value == "asdf"; }));
  }
}

TEST_CASE("UnorderedApproxVecMatches")
{
  using V = std::vector<vm::vec3d>;
  CHECK_THAT((V{{1, 1, 1}}), UnorderedApproxVecMatches(V{{1.01, 1.01, 1.01}}, 0.02));
  CHECK_THAT(
    (V{{0, 0, 0}, {1, 1, 1}}),
    UnorderedApproxVecMatches(V{{1.01, 1.01, 1.01}, {-0.01, -0.01, -0.01}}, 0.02));

  CHECK_THAT(
    (V{{1, 1, 1}}),
    !UnorderedApproxVecMatches(
      V{{1.01, 1.01, 1.01}, {1, 1, 1}}, 0.02)); // different number of elements
  CHECK_THAT(
    (V{{1, 1, 1}}), !UnorderedApproxVecMatches(V{{1.05, 1.01, 1.01}}, 0.02)); // too far
}

} // namespace tb
