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

#include "Model/CompilationConfig.h"
#include "Model/CompilationTask.h"
#include "Model/CompilationProfile.h"

namespace TrenchBroom {
    namespace Model {
        struct CompilationConfigObserver {
            size_t callCount = 0;

            void onCall() {
                ++callCount;
            }
        };

        TEST_CASE("CompilationConfigTest.configDidChange", "[CompilationProfileTest]") {
            CompilationConfigObserver o;

            auto tasks = std::vector<std::unique_ptr<CompilationTask>>();
            tasks.push_back(std::move(std::make_unique<CompilationExportMap>("target spec 1")));
            tasks.push_back(std::move(std::make_unique<CompilationCopyFiles>("src", "dest")));
            tasks.push_back(std::move(std::make_unique<CompilationRunTool>("tool", "args")));

            auto config = CompilationConfig();
            config.configDidChange.addObserver(&o, &CompilationConfigObserver::onCall);
            CHECK(o.callCount == 0);

            config.addProfile(std::make_unique<CompilationProfile>("name", "workDir", std::move(tasks)));
            CHECK(o.callCount == 1);

            SECTION("profile list") {
                SECTION("addition") {
                    config.addProfile(std::make_unique<CompilationProfile>("x", "y"));
                    CHECK(o.callCount == 2);
                }
                SECTION("removal") {
                    config.removeProfile(0);
                    CHECK(o.callCount == 2);
                }
            }

            SECTION("profile") {
                SECTION("renaming") {
                    config.profile(0)->setName("something");
                    CHECK(o.callCount == 2);
                }
                SECTION("setting work dir") {
                    config.profile(0)->setWorkDirSpec("a");
                    CHECK(o.callCount == 2);
                }
            }

            SECTION("task list") {
                SECTION("addition") {
                    config.profile(0)->addTask(std::make_unique<CompilationExportMap>("export 2"));
                    CHECK(o.callCount == 2);
                }
                SECTION("removal") {
                    config.profile(0)->removeTask(0);
                    CHECK(o.callCount == 2);
                }
            }

            SECTION("task") {
                REQUIRE(config.profile(0)->taskCount() == 3);

                auto* exportMapTask = dynamic_cast<CompilationExportMap *>(config.profile(0)->task(0));
                auto* copyFilesTask = dynamic_cast<CompilationCopyFiles *>(config.profile(0)->task(1));
                auto* runToolTask = dynamic_cast<CompilationRunTool *>(config.profile(0)->task(2));

                REQUIRE(exportMapTask != nullptr);
                REQUIRE(copyFilesTask != nullptr);
                REQUIRE(runToolTask != nullptr);

                SECTION("CompilationExportMap") {
                    SECTION("setTargetSpec") {
                        exportMapTask->setTargetSpec("changed");
                        CHECK(o.callCount == 2);
                    }
                    SECTION("setTargetSpec unchanged") {
                        exportMapTask->setTargetSpec(exportMapTask->targetSpec());
                        CHECK(o.callCount == 1);
                    }
                }

                SECTION("CompilationCopyFiles") {
                    SECTION("setTargetSpec") {
                        copyFilesTask->setTargetSpec("changed");
                        CHECK(o.callCount == 2);
                    }SECTION("setSourceSpec") {
                        copyFilesTask->setSourceSpec("changed");
                        CHECK(o.callCount == 2);
                    }
                }

                SECTION("CompilationRunTool") {
                    SECTION("setToolSpec") {
                        runToolTask->setToolSpec("changed");
                        CHECK(o.callCount == 2);
                    }SECTION("setParameterSpec") {
                        runToolTask->setParameterSpec("changed");
                        CHECK(o.callCount == 2);
                    }
                }
            }

            config.configDidChange.removeObserver(&o, &CompilationConfigObserver::onCall);
        }
    }
}
