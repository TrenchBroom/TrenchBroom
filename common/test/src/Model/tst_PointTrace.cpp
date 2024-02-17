/*
 Copyright (C) 2021 Kristian Duske

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

#include "Error.h"
#include "Model/PointTrace.h"

#include "vm/vec.h"
#include "vm/vec_io.h"

#include <sstream>

#include "Catch2.h"

namespace TrenchBroom::Model
{
TEST_CASE("PointTrace")
{
  const auto points = std::vector<vm::vec3f>{
    {1, 1, 1},
    {1, 1, 2},
    {1, 2, 2},
  };

  auto trace = PointTrace{points};
  CHECK(trace.points() == points);

  CHECK(trace.hasNextPoint());
  CHECK_FALSE(trace.hasPreviousPoint());
  CHECK(trace.currentPoint() == vm::vec3f{1, 1, 1});
  CHECK(trace.currentDirection() == vm::vec3f{0, 0, 1});

  trace.advance();

  CHECK(trace.hasNextPoint());
  CHECK(trace.hasPreviousPoint());
  CHECK(trace.currentPoint() == vm::vec3f{1, 1, 2});
  CHECK(trace.currentDirection() == vm::vec3f{0, 1, 0});

  trace.advance();

  CHECK_FALSE(trace.hasNextPoint());
  CHECK(trace.hasPreviousPoint());
  CHECK(trace.currentPoint() == vm::vec3f{1, 2, 2});
  CHECK(trace.currentDirection() == vm::vec3f{0, 1, 0});

  trace.advance();
  CHECK(trace.currentPoint() == vm::vec3f{1, 2, 2});

  trace.retreat();
  CHECK(trace.currentPoint() == vm::vec3f{1, 1, 2});

  trace.retreat();
  CHECK(trace.currentPoint() == vm::vec3f{1, 1, 1});

  trace.retreat();
  CHECK(trace.currentPoint() == vm::vec3f{1, 1, 1});
}

TEST_CASE("loadPointFile")
{
  using T = std::tuple<std::string, Result<PointTrace>>;
  // clang-format off
  const auto
  [file,       expectedTrace] = GENERATE(values<T>({
  {R"()",      Error{"PointFile must contain at least two points"}},
  {R"(asdf)",  Error{"PointFile must contain at least two points"}},
  {R"(1)",     Error{"PointFile must contain at least two points"}},
  {R"(1 2)",   Error{"PointFile must contain at least two points"}},
  {R"(1 2 3)", Error{"PointFile must contain at least two points"}},
  {R"(
    1 2 3
    4 5 6
  )",          
               PointTrace{{
                 {1, 2, 3},
                 {4, 5, 6},
               }}},
  {R"(
    0 0 1
    0 0 1
    4 5 6
    4 5 6
  )",          
               PointTrace{{
                 {0, 0, 1},
                 {4, 5, 6},
               }}},
  {R"(
    0 0 1
    0 0 2
    0 0 3
    4 5 6
  )",          
               PointTrace{{
                 {0, 0, 1},
                 {0, 0, 3},
                 {4, 5, 6},
               }}},
  }));
  // clang-format on

  auto stream = std::istringstream{file};
  CHECK(loadPointFile(stream) == expectedTrace);
}
} // namespace TrenchBroom::Model
