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

#include "Exceptions.h"
#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"
#include "View/Grid.h"

#include <kdl/result.h>

#include <vecmath/approx.h>
#include <vecmath/polygon.h>
#include <vecmath/segment.h>

#include <cmath>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        static const vm::bbox3 worldBounds(8192.0);

        TEST_CASE("GridTest.size", "[GridTest]") {
            for (int i = Grid::MinSize; i < Grid::MaxSize; ++i)
                CHECK(Grid(i).size() == i);
        }

        TEST_CASE("GridTest.actualSizeInteger", "[GridTest]") {
            for (int i = 0; i < Grid::MaxSize; ++i) {
                const int actualSize = static_cast<int>(std::pow(2, i));
                CHECK(Grid(i).actualSize() == actualSize);
            }
        }

        TEST_CASE("GridTest.actualSizeSubInteger", "[GridTest]") {
            CHECK(Grid(-1).actualSize() == 0.5);
            CHECK(Grid(-2).actualSize() == 0.25);
            CHECK(Grid(-3).actualSize() == 0.125);
        }

        TEST_CASE("GridTest.changeSize", "[GridTest]") {
            Grid g(0);
            g.incSize();
            CHECK(g.size() == 1);
            g.decSize();
            CHECK(g.size() == 0);
            g.decSize();
            CHECK(g.size() == -1);

            g.setSize(4);
            CHECK(g.size() == 4);
        }

        TEST_CASE("GridTest.offsetScalars", "[GridTest]") {
            CHECK(Grid(2u).offset(0.0) == vm::approx(0.0));
            CHECK(Grid(2u).offset(0.3) == vm::approx(0.3));
            CHECK(Grid(2u).offset(-0.3) == vm::approx(-0.3));

            CHECK(Grid(2u).offset(4.0) == vm::approx(0.0));
            CHECK(Grid(2u).offset(4.3) == vm::approx(0.3));
            CHECK(Grid(2u).offset(-4.3) == vm::approx(-0.3));

            CHECK(Grid(2u).offset(3.0) == vm::approx(-1.0));
            CHECK(Grid(2u).offset(5.0) == vm::approx(1.0));
        }

        TEST_CASE("GridTest.snapScalars", "[GridTest]") {
            CHECK(Grid(-1).snap(0.0) == vm::approx(0.0));
            CHECK(Grid(-1).snap(0.1) == vm::approx(0.0));
            CHECK(Grid(-1).snap(0.24) == vm::approx(0.0));
            CHECK(Grid(-1).snap(0.25) == vm::approx(0.5));
            CHECK(Grid(-1).snap(0.7) == vm::approx(0.5));

            CHECK(Grid(0u).snap(0.0) == vm::approx(0.0));
            CHECK(Grid(0u).snap(0.3) == vm::approx(0.0));
            CHECK(Grid(0u).snap(0.49) == vm::approx(0.0));
            CHECK(Grid(0u).snap(0.5) == vm::approx(1.0));
            CHECK(Grid(0u).snap(1.3) == vm::approx(1.0));

            CHECK(Grid(2u).snap(0.0) == vm::approx(0.0));
            CHECK(Grid(2u).snap(1.999) == vm::approx(0.0));
            CHECK(Grid(2u).snap(2.0) == vm::approx(4.0));
            CHECK(Grid(2u).snap(-1.999) == vm::approx(0.0));
            CHECK(Grid(2u).snap(-2.0) == vm::approx(-4.0));

            CHECK(Grid(2u).snapUp(0.0, false) == vm::approx(0.0));
            CHECK(Grid(2u).snapUp(1.999, false) == vm::approx(4.0));
            CHECK(Grid(2u).snapUp(2.0, false) == vm::approx(4.0));
            CHECK(Grid(2u).snapUp(-1.999, false) == vm::approx(0.0));
            CHECK(Grid(2u).snapUp(-2.0, false) == vm::approx(0.0));
            CHECK(Grid(2u).snapUp(-4.0, false) == vm::approx(-4.0));

            CHECK(Grid(2u).snapUp(0.0, true) == vm::approx(4.0));
            CHECK(Grid(2u).snapUp(1.999, true) == vm::approx(4.0));
            CHECK(Grid(2u).snapUp(2.0, true) == vm::approx(4.0));
            CHECK(Grid(2u).snapUp(4.0, true) == vm::approx(8.0));
            CHECK(Grid(2u).snapUp(-1.999, true) == vm::approx(0.0));
            CHECK(Grid(2u).snapUp(-2.0, true) == vm::approx(0.0));
            CHECK(Grid(2u).snapUp(-4.0, true) == vm::approx(0.0));
        }

        TEST_CASE("GridTest.snapOnLine", "[GridTest]") {
            const vm::line3d X(vm::vec3d(5.0, 0.0, 0.0), vm::vec3d::pos_x());

            CHECK(Grid(2u).snap(vm::vec3d::zero(), X) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(1.0, 0.0, 0.0), X) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(1.0, 1.0, 0.0), X) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(3.0, 1.0, 0.0), X) == vm::approx(vm::vec3d(4.0, 0.0, 0.0)));
            CHECK(Grid(2u).snap(vm::vec3(3.0, 1.0, 2.0), X) == vm::approx(vm::vec3d(4.0, 0.0, 0.0)));

            const vm::line3d L(vm::vec3d::zero(), normalize(vm::vec3d(1.0, 2.0, 0.0)));
            CHECK(Grid(2u).snap(vm::vec3d::zero(), L) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(1.0, 0.0, 0.0), L) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(10.0, 0.0, 0.0), L) == vm::approx(vm::vec3d(2.0, 4.0, 0.0)));
            CHECK(Grid(2u).snap(vm::vec3(7.5, 0.0, 0.0), L) == vm::approx(vm::vec3d(2.0, 4.0, 0.0)));
        }

        TEST_CASE("GridTest.snapOnEdge", "[GridTest]") {
            const vm::segment3d E(vm::vec3d::zero(), vm::vec3d(1.0, 2.0, 0.0) * 2.0);
            CHECK(Grid(2u).snap(vm::vec3d::zero(), E) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(1.0, 0.0, 0.0), E) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3(10.0, 0.0, 0.0), E) == vm::approx(vm::vec3d(2.0, 4.0, 0.0)));
            CHECK(Grid(2u).snap(vm::vec3(7.5, 0.0, 0.0), E) == vm::approx(vm::vec3d(2.0, 4.0, 0.0)));
            CHECK(vm::is_nan(Grid(2u).snap(vm::vec3(20.0, 0.0, 0.0), E)));
            CHECK(vm::is_nan(Grid(2u).snap(vm::vec3(-10.0, 0.0, 0.0), E)));
        }

        TEST_CASE("GridTest.snapOnQuad", "[GridTest]") {
            const vm::polygon3d quad {
                    vm::vec3d(-9.0, -9.0, 0.0),
                    vm::vec3d(+9.0, -9.0, 0.0),
                    vm::vec3d(+9.0, +9.0, 0.0),
                    vm::vec3d(-9.0, +9.0, 0.0)
            };

            CHECK(Grid(2u).snap(vm::vec3d(0.0, 0.0, 0.0), quad, vm::vec3d::pos_z()) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3d(1.0, 1.0, 0.0), quad, vm::vec3d::pos_z()) == vm::approx(vm::vec3d::zero()));
            CHECK(Grid(2u).snap(vm::vec3d(1.0, 1.0, 1.0), quad, vm::vec3d::pos_z()) == vm::approx(vm::vec3d::zero()));

            CHECK(Grid(2u).snap(vm::vec3d(10.0, 3.0, 1.0), quad, vm::vec3d::pos_z()) == vm::approx(vm::vec3d(9.0, 4.0, 0.0)));
            CHECK(Grid(2u).snap(vm::vec3d(10.0, -2.0, 1.0), quad, vm::vec3d::pos_z()) == vm::approx(vm::vec3d(9.0, -4.0, 0.0)));
        }

        TEST_CASE("GridTest.moveDeltaForPoint", "[GridTest]") {
            const auto grid16 = Grid(4);

            const auto pointOffGrid = vm::vec3d(17, 17, 17);
            const auto inputDelta = vm::vec3d(1, 1, 7); // moves point to (18, 18, 24)
            const auto pointOnGrid = vm::vec3d(17, 17, 32);

            CHECK(pointOffGrid + grid16.moveDeltaForPoint(pointOffGrid, inputDelta) == pointOnGrid);
        }

        TEST_CASE("GridTest.moveDeltaForPoint_SubInteger", "[GridTest]") {
            const auto grid05 = Grid(-1);

            const auto pointOffGrid = vm::vec3d(0.51, 0.51, 0.51);
            const auto inputDelta = vm::vec3d(0.01, 0.01, 0.30); // moves point to (0.52, 0.52, 0.81)
            const auto pointOnGrid = vm::vec3d(0.51, 0.51, 1.0);

            CHECK(pointOffGrid + grid05.moveDeltaForPoint(pointOffGrid, inputDelta) == pointOnGrid);
        }

        TEST_CASE("GridTest.moveDeltaForPoint_SubInteger2", "[GridTest]") {
            const auto grid05 = Grid(-1);

            const auto pointOffGrid = vm::vec3d(0.51, 0.51, 0.51);
            const auto inputDelta = vm::vec3d(0.01, 0.01, 1.30); // moves point to (0.52, 0.52, 1.81)
            const auto pointOnGrid = vm::vec3d(0.51, 0.51, 2.0);

            CHECK(pointOffGrid + grid05.moveDeltaForPoint(pointOffGrid, inputDelta) == pointOnGrid);
        }

        static Model::Brush makeCube128() {
            Assets::Texture texture("testTexture", 64, 64);
            Model::WorldNode world(Model::Entity(), Model::MapFormat::Standard);
            Model::BrushBuilder builder(world.mapFormat(), worldBounds);
            return builder.createCube(128.0, "").value();
        }

        TEST_CASE("GridTest.moveDeltaForFace", "[GridTest]") {
            const auto grid16 = Grid(4);

            const Model::Brush cube = makeCube128();
            const auto topFaceIndex = cube.findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);
            const Model::BrushFace& topFace = cube.face(*topFaceIndex);

            CHECK(topFace.boundsCenter().z() == vm::approx(64.0));

            // try to move almost 4 grid increments up -> snaps to 3
            CHECK(grid16.moveDelta(topFace, vm::vec3(0, 0, 63)) == vm::approx(vm::vec3(0,0,48)));
            CHECK(grid16.moveDelta(topFace, vm::vec3(0, 0, 64)) == vm::approx(vm::vec3(0,0,64)));
            CHECK(grid16.moveDelta(topFace, vm::vec3(0, 0, 65)) == vm::approx(vm::vec3(0,0,64)));
        }

        TEST_CASE("GridTest.moveDeltaForFace_SubInteger", "[GridTest]") {
            const auto grid05 = Grid(-1);

            const Model::Brush cube = makeCube128();
            const auto topFaceIndex = cube.findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);
            const Model::BrushFace& topFace = cube.face(*topFaceIndex);

            CHECK(topFace.boundsCenter().z() == vm::approx(64.0));

            // try to move almost 4 grid increments up -> snaps to 3
            CHECK(grid05.moveDelta(topFace, vm::vec3(0, 0, 1.9)) == vm::approx(vm::vec3(0,0,1.5)));
            CHECK(grid05.moveDelta(topFace, vm::vec3(0, 0, 2)) == vm::approx(vm::vec3(0,0,2)));
            CHECK(grid05.moveDelta(topFace, vm::vec3(0, 0, 2.1)) == vm::approx(vm::vec3(0,0,2)));
        }

        static vm::ray3 make_ray_from_to(const vm::vec3& from, const vm::vec3& to) {
            return vm::ray3(from, vm::normalize(to - from));
        }

        TEST_CASE("GridTest.moveDeltaForBounds", "[GridTest]") {
            const auto grid16 = Grid(4);

            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(95, 100, 105));
            SECTION("drop to floor") {
                const auto floor = vm::plane3(vm::vec3::zero(), vm::vec3::pos_z());

                SECTION("camera looking towards +x +y") {
                    const auto pickRay = make_ray_from_to(vm::vec3(512, 512, 200), vm::vec3(1024 - 8, 1024 - 8, 0));

                    // Snaps towards the camera
                    CHECK(grid16.moveDeltaForBounds(floor, box, worldBounds, pickRay) == vm::approx(vm::vec3(1024 - 16, 1024 - 16, 0)));
                }

                SECTION("camera looking towards -x -y") {
                    const auto pickRay = make_ray_from_to(vm::vec3(512, 512, 200), vm::vec3(8, 8, 0));

                    // Note, the box corner is rounded towards the camera (vm::vec3(8, 8, 0) -> vm::vec3(16, 16, 0))
                    const auto snappedBoxCorner = vm::vec3(16, 16, 0);
                    // But the box orientation is pushed away from the camera so the snapped box mins are:
                    const auto newBoxMin = snappedBoxCorner - vm::vec3(box.size().x(), box.size().y(), 0.0);

                    CHECK(grid16.moveDeltaForBounds(floor, box, worldBounds, pickRay) == newBoxMin);
                }
            }

            SECTION("drop to ceiling") {
                const FloatType ceilHeight = 512.0;

                const auto ceil = vm::plane3(vm::vec3(0, 0, ceilHeight), vm::vec3::neg_z());
                const auto pickRay = make_ray_from_to(vm::vec3(50, 50, 200), vm::vec3(1024 - 8, 1024 - 8, ceilHeight));

                // Snaps towards the camera
                const auto snappedBoxCorner = vm::vec3(1024 - 16, 1024 - 16, ceilHeight);
                const auto newBoxMin = snappedBoxCorner - vm::vec3(0.0, 0.0, box.size().z());

                CHECK(grid16.moveDeltaForBounds(ceil, box, worldBounds, pickRay) == newBoxMin);
            }

            SECTION("drop onto a sub-grid platform") {
                const auto subGridPlatform = vm::plane3(vm::vec3(0, 0, 4), vm::vec3::pos_z());
                const auto pickRay = make_ray_from_to(vm::vec3(0, 0, 200), vm::vec3(17, 17, 4));

                // We allow a sub-grid result here because it's a flat plane
                CHECK(grid16.moveDeltaForBounds(subGridPlatform, box, worldBounds, pickRay) == vm::approx(vm::vec3(16, 16, 4)));
            }

            SECTION("drop onto a slope") {
                const auto [ok, slope] = vm::from_points(vm::vec3::zero(), vm::vec3(0, 100, 5), vm::vec3(100, 0, 0));
                REQUIRE(ok);
                CHECK(slope.normal.z() > 0.0);

                const auto pickRay = make_ray_from_to(vm::vec3(0, 0, 200), vm::vec3(17, 17, 0));

                // Float above the sloped plane
                CHECK(grid16.moveDeltaForBounds(slope, box, worldBounds, pickRay) == vm::approx(vm::vec3(16, 16, 16)));
            }
        }
    }
}
