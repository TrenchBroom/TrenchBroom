/*
 Copyright (C) 2010 Kristian Duske

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

#include "Exceptions.h"
#include "Result.h"
#include "TestLogger.h"
#include "el/Exceptions.h"
#include "mdl/AssetUtils.h"

#include <optional>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("AssetUtils")
{
  SECTION("safeGetModelSpecification")
  {
    TestLogger logger;

    std::optional<ModelSpecification> actual;

    SECTION("Returning a specification logs nothing")
    {
      const auto expected = ModelSpecification{"test/test", 1, 2};

      // regular execution is fine
      SECTION("Regular execution")
      {
        CHECK_NOTHROW(actual = safeGetModelSpecification(logger, "", [&]() {
                        return Result<ModelSpecification>{expected};
                      }));
        CHECK(logger.countMessages() == 0u);
        CHECK(actual.has_value());
        CHECK(*actual == expected);
      }
    }

    SECTION("Returning an error logs and returns an empty model spec")
    {
      CHECK_NOTHROW(actual = safeGetModelSpecification(logger, "", []() {
                      return Result<ModelSpecification>{Error{"some error"}};
                    }));
      CHECK(logger.countMessages() == 1u);
      CHECK(logger.countMessages(LogLevel::Error) == 1u);
      CHECK(actual.has_value());
      CHECK(*actual == ModelSpecification());
    }
  }
}

} // namespace tb::mdl
