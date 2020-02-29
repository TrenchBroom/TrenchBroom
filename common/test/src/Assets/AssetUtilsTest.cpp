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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TestLogger.h"

#include "Exceptions.h"
#include "Assets/AssetUtils.h"
#include "IO/Path.h"

#include <optional>

namespace TrenchBroom {
    namespace Assets {
        TEST_CASE("AssetUtilsTest.safeGetModelSpecification", "[AssetUtilsTest]") {
            TestLogger logger;

            const auto expected = ModelSpecification(IO::Path("test/test"), 1, 2);
            std::optional<ModelSpecification> actual;
            
            // regular execution is fine
            ASSERT_NO_THROW(actual = safeGetModelSpecification(logger, "", [&]() {
                return expected;
            }));
            ASSERT_EQ(0u, logger.countMessages());
            ASSERT_TRUE(actual.has_value());
            ASSERT_EQ(expected, *actual);
            
            // only ELExceptions are caught, and nothing is logged
            ASSERT_THROW(safeGetModelSpecification(logger, "", []() -> ModelSpecification {
                throw AssetException();
            }), AssetException);
            ASSERT_EQ(0u, logger.countMessages());

            // throwing an EL exception logs and returns an empty model spec
            ASSERT_NO_THROW(actual = safeGetModelSpecification(logger, "", []() -> ModelSpecification {
                throw EL::Exception();
            }));
            ASSERT_EQ(1u, logger.countMessages());
            ASSERT_EQ(1u, logger.countMessages(LogLevel::Error));
            ASSERT_TRUE(actual.has_value());
            ASSERT_EQ(ModelSpecification(), *actual);
        }
    }
}
