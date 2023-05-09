/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Assets/AssetUtils.h"
#include "Exceptions.h"
#include "TestLogger.h"

#include <filesystem>
#include <optional>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Assets
{
TEST_CASE("AssetUtilsTest.safeGetModelSpecification")
{
  TestLogger logger;

  const auto expected = ModelSpecification("test/test", 1, 2);
  std::optional<ModelSpecification> actual;

  // regular execution is fine
  SECTION("Regular execution")
  {
    CHECK_NOTHROW(
      actual = safeGetModelSpecification(logger, "", [&]() { return expected; }));
    CHECK(logger.countMessages() == 0u);
    CHECK(actual.has_value());
    CHECK(*actual == expected);
  }

  // only ELExceptions are caught, and nothing is logged
  SECTION("Only ELExceptions are caught, and nothing is logged")
  {
    CHECK_THROWS_AS(
      safeGetModelSpecification(
        logger, "", []() -> ModelSpecification { throw AssetException(); }),
      AssetException);
    CHECK(logger.countMessages() == 0u);
  }

  // throwing an EL exception logs and returns an empty model spec
  SECTION("Throwing an EL exception logs and returns an empty model spec")
  {
    CHECK_NOTHROW(
      actual = safeGetModelSpecification(
        logger, "", []() -> ModelSpecification { throw EL::Exception(); }));
    CHECK(logger.countMessages() == 1u);
    CHECK(logger.countMessages(LogLevel::Error) == 1u);
    CHECK(actual.has_value());
    CHECK(*actual == ModelSpecification());
  }
}
} // namespace Assets
} // namespace TrenchBroom
