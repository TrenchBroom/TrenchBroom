/*
 Copyright (C) 2020-2020 Eric Wasylishen

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

#include "IO/Path.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

#include <catch2/catch.hpp>

namespace TrenchBroom {
    namespace Model {
        struct GameEngineConfigObserver {
            size_t callCount = 0;

            void onCall() {
                ++callCount;
            }
        };

        TEST_CASE("GameEngineConfigTest.configDidChange", "[GameEngineConfigTest]") {
            GameEngineConfigObserver o;

            auto config = GameEngineConfig();
            config.configDidChange.addObserver(&o, &GameEngineConfigObserver::onCall);
            CHECK(o.callCount == 0);

            config.addProfile(std::make_unique<GameEngineProfile>("engine name", IO::Path(), "params"));
            CHECK(o.callCount == 1);

            SECTION("profile list") {
                SECTION("addition") {
                    config.addProfile(std::make_unique<GameEngineProfile>("engine name 2", IO::Path(), "params"));
                    CHECK(o.callCount == 2);
                }
                SECTION("removal") {
                    config.removeProfile(0);
                    CHECK(o.callCount == 2);
                }
            }

            SECTION("profile") {
                SECTION("setName") {
                    config.profile(0)->setName("something");
                    CHECK(o.callCount == 2);
                }
                SECTION("setName unchanged") {
                    config.profile(0)->setName(config.profile(0)->name());
                    CHECK(o.callCount == 1);
                }
                SECTION("setPath") {
                    config.profile(0)->setPath(IO::Path("xyz"));
                    CHECK(o.callCount == 2);
                }
                SECTION("setParameterSpec") {
                    config.profile(0)->setParameterSpec("xyz");
                    CHECK(o.callCount == 2);
                }
            }

            config.configDidChange.removeObserver(&o, &GameEngineConfigObserver::onCall);
        }
    }
}
