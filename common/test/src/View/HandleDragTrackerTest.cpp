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

#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Renderer/OrthographicCamera.h"
#include "View/HandleDragTracker.h"
#include "View/Grid.h"

#include <vecmath/approx.h>
#include <vecmath/line.h>
#include <vecmath/line_io.h>
#include <vecmath/plane.h>
#include <vecmath/plane_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <tuple>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        struct TestDelegateData {
            std::vector<std::tuple<vm::vec3, vm::vec3>> initializeArguments;
            HandlePositionProposer initialGetHandlePositionToReturn;

            std::vector<std::tuple<DragState, vm::vec3>> dragArguments;
            DragStatus dragStatusToReturn{DragStatus::Continue};

            std::vector<DragState> endArguments;
            std::vector<DragState> cancelArguments;

            std::vector<DragState> modifierKeyChangeArguments;
            std::optional<UpdateDragConfig> updateDragConfigToReturn;

            std::vector<DragState> mouseScrollArguments;

            TestDelegateData(HandlePositionProposer i_initialGetHandlePositionToReturn) :
            initialGetHandlePositionToReturn{std::move(i_initialGetHandlePositionToReturn)} {}
        };

        struct TestDelegate : public HandleDragTrackerDelegate {
            TestDelegateData& data;

            TestDelegate(TestDelegateData& i_data) :
            data{i_data} {}

            HandlePositionProposer start(const InputState&, const vm::vec3& initialHandlePosition, const vm::vec3& handleOffset) {
                data.initializeArguments.emplace_back(initialHandlePosition, handleOffset);
                return data.initialGetHandlePositionToReturn;
            }

            DragStatus drag(const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) {
                data.dragArguments.emplace_back(dragState, proposedHandlePosition);
                return data.dragStatusToReturn;
            }

            void end(const InputState&, const DragState& dragState) {
                data.endArguments.emplace_back(dragState);
            }

            void cancel(const DragState& dragState) {
                data.cancelArguments.emplace_back(dragState);
            }

            std::optional<UpdateDragConfig> modifierKeyChange(const InputState&, const DragState& dragState) {
                data.modifierKeyChangeArguments.emplace_back(dragState);
                return data.updateDragConfigToReturn;
            }

            void mouseScroll(const InputState&, const DragState& dragState) {
                data.mouseScrollArguments.emplace_back(dragState);
            }
        };

        auto makeHandleTracker(TestDelegateData& data, const vm::vec3& initialHandlePosition, const vm::vec3& handleOffset) {
            return HandleDragTracker<TestDelegate>{TestDelegate{data}, InputState{}, initialHandlePosition, handleOffset};
        }

        TEST_CASE("RestrictedDragTracker.constructor") {
            GIVEN("A delegate") {
                const auto initialHandlePosition = vm::vec3{1, 1, 1};
                const auto handleOffset = vm::vec3{0, 0, 1};

                auto data = TestDelegateData{
                    makeHandlePositionProposer(
                        // always returns the same handle position
                        [](const auto&) { return vm::vec3{2, 2, 2}; },
                        makeIdentityHandleSnapper())
                };

                auto tracker = makeHandleTracker(data, initialHandlePosition, handleOffset);

                THEN("The initial handle position was passed to initialize") {
                    CHECK(data.initializeArguments == std::vector<std::tuple<vm::vec3, vm::vec3>>{
                        {initialHandlePosition, handleOffset}
                    });

                    AND_THEN("The initial handle position is passed to drag for the initial and the last handle position") {
                        tracker.drag(InputState{});

                        CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                            {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2}},
                        });
                    }
                }
            }
        }

        TEST_CASE("RestrictedDragTracker.drag") {
            GIVEN("A drag tracker") {
                const auto initialHandlePosition = vm::vec3{1, 1, 1};
                auto handlePositionToReturn = vm::vec3{};

                auto data = TestDelegateData{
                    makeHandlePositionProposer(
                        // always returns the same hit position
                        [&](const auto&) { return handlePositionToReturn; },
                        makeIdentityHandleSnapper())
                };

                auto tracker = makeHandleTracker(data, initialHandlePosition, vm::vec3::zero());

                WHEN("drag is called for the first time after the drag started") {
                    handlePositionToReturn = vm::vec3{2, 2, 2};
                    REQUIRE(tracker.drag(InputState{}));

                    THEN("drag got the initial and the next handle positions") {
                        CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                            {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 0}}, vm::vec3{2, 2, 2}},
                        });

                        AND_WHEN("drag is called again") {
                            handlePositionToReturn = vm::vec3{3, 3, 3};
                            REQUIRE(tracker.drag(InputState{}));

                            THEN("drag got the last and the next handle positions") {
                                CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                    {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 0}}, vm::vec3{2, 2, 2}},
                                    {{vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 0}}, vm::vec3{3, 3, 3}},
                                });
                            }
                        }
                    }
                }

                WHEN("drag returns drag status deny") {
                    handlePositionToReturn = vm::vec3{2, 2, 2};
                    data.dragStatusToReturn = DragStatus::Deny;
                    REQUIRE(tracker.drag(InputState{}));

                    THEN("drag got the initial and the next handle positions") {
                        CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                            {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 0}}, vm::vec3{2, 2, 2}},
                        });

                        AND_WHEN("drag is called again") {
                            handlePositionToReturn = vm::vec3{3, 3, 3};
                            REQUIRE(tracker.drag(InputState{}));

                            THEN("drag got the initial handle position for the last handle position again") {
                                CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                    {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 0}}, vm::vec3{2, 2, 2}},
                                    {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 0}}, vm::vec3{3, 3, 3}},
                                });
                            }
                        }
                    }
                }

                WHEN("drag returns drag status cancel") {
                    handlePositionToReturn = vm::vec3{2, 2, 2};
                    data.dragStatusToReturn = DragStatus::End;
                    const auto dragResult = tracker.drag(InputState{});

                    THEN("the drag tracker returns false") {
                        CHECK_FALSE(dragResult);
                    }
                }
            }
        }

        TEST_CASE("RestrictedDragTracker.handlePositionComputations") {
            const auto initialHandlePosition = vm::vec3{1, 1, 1};

            auto getHandlePositionArguments = std::vector<std::tuple<DragState, vm::vec3>>{};
            auto handlePositionToReturn = vm::vec3{};

            GIVEN("A drag tracker") {
                auto data = TestDelegateData{
                    makeHandlePositionProposer(
                        // returns the handle position set above
                        [&](const InputState&) { return handlePositionToReturn; },
                        // returns the proposed handle position, but records the arguments
                        [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
                            getHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
                            return proposedHandlePosition;
                        })
                };

                auto tracker = makeHandleTracker(data, initialHandlePosition, vm::vec3{0, 0, 1});

                WHEN("drag is called for the first time") {
                    handlePositionToReturn = vm::vec3{2, 2, 2};
                    REQUIRE(tracker.drag(InputState{}));

                    THEN("getHandlePosition is called with the expected arguments") {
                        CHECK(getHandlePositionArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                            {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2}},
                        });

                        AND_THEN("The new handle position was passed to the delegate's drag function") {
                            CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2}},
                            });
                        }
                    }

                    AND_WHEN("drag is called again") {
                        handlePositionToReturn = vm::vec3{3, 3, 3};
                        REQUIRE(tracker.drag(InputState{}));

                        THEN("getHandlePosition is called with the expected arguments") {
                            CHECK(getHandlePositionArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2}},
                                {{vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}, vm::vec3{3, 3, 3}},
                            });


                            AND_THEN("The hit position was passed to the delegate's drag function") {
                                CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                    {{vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2}},
                                    {{vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}, vm::vec3{3, 3, 3}},
                                });
                            }
                        }
                    }
                }
            }
        }

        TEST_CASE("RestrictedDragTracker.modifierKeyChange") {
            const auto initialHandlePosition = vm::vec3{1, 1, 1};
            const auto handleOffset = vm::vec3{0, 0, 1};

            auto initialGetHandlePositionArguments = std::vector<std::tuple<DragState, vm::vec3>>{};

            GIVEN("A delegate that returns null from modifierKeyChange") {
                auto data = TestDelegateData{
                    makeHandlePositionProposer(
                        // returns a constant handle position
                        [&](const InputState&) { return vm::vec3{2, 2, 2}; },
                        // returns the proposed handle position, but records the arguments
                        [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
                            initialGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
                            return proposedHandlePosition;
                        })
                };

                auto tracker = makeHandleTracker(data, initialHandlePosition, handleOffset);

                tracker.drag(InputState{});
                REQUIRE(initialGetHandlePositionArguments.size() == 1);

                WHEN("A modifier key change is notified") {
                    tracker.modifierKeyChange(InputState{});
                    
                    THEN("The drag state are passed to the delegate") {
                        CHECK(data.modifierKeyChangeArguments == std::vector<DragState>{
                            {vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}
                        });
                        
                        AND_THEN("The next call to drag uses the initial drag config") {
                            tracker.drag(InputState{});
                            CHECK(initialGetHandlePositionArguments.size() == 2);
                        }
                    }
                }
            }

            GIVEN("A delegate that returns a new drag config from modifierKeyChange") {
                auto otherGetHandlePositionArguments = std::vector<std::tuple<DragState, vm::vec3>>{};
                auto otherHitPositionToReturn = vm::vec3{};

                auto data = TestDelegateData{
                    makeHandlePositionProposer(
                        // returns a constant hit position
                        [&](const InputState&) { return vm::vec3{2, 2, 2}; },
                        // returns the proposed handle position, but records the arguments
                        [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
                            initialGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
                            return proposedHandlePosition;
                        })
                };

                data.updateDragConfigToReturn = UpdateDragConfig{
                    makeHandlePositionProposer(
                        // returns a constant hit position
                        [&](const InputState&) { return otherHitPositionToReturn; },
                        // returns the proposed handle position, but records the arguments
                        [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
                            otherGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
                            return proposedHandlePosition;
                        }),
                    ResetInitialHandlePosition::Keep
                };

                auto tracker = makeHandleTracker(data, initialHandlePosition, vm::vec3{0, 0, 1});

                tracker.drag(InputState{});
                REQUIRE(initialGetHandlePositionArguments.size() == 1);
                REQUIRE(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                    { {vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2} },
                });

                WHEN("A modifier key change is notified") {
                    otherHitPositionToReturn = vm::vec3{3, 3, 3};
                    tracker.modifierKeyChange(InputState{});
                    
                    THEN("The drag state was passed to the delegate") {
                        CHECK(data.modifierKeyChangeArguments == std::vector<DragState>{
                            {vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}
                        });

                        AND_THEN("A synthetic drag to the new handle position happens using the other drag config") {
                            CHECK(initialGetHandlePositionArguments.size() == 1);
                            CHECK(otherGetHandlePositionArguments.size() == 1);

                            CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                { {vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2} },
                                { {vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}, vm::vec3{3, 3, 3} },
                            });
                        }
                        
                        AND_WHEN("drag is called again") {
                            otherHitPositionToReturn = vm::vec3{4, 4, 4};
                            tracker.drag(InputState{});
                            
                            AND_THEN("The other handle position is passed") {
                                CHECK(data.dragArguments == std::vector<std::tuple<DragState, vm::vec3>>{
                                    { {vm::vec3{1, 1, 1}, vm::vec3{1, 1, 1}, vm::vec3{0, 0, 1}}, vm::vec3{2, 2, 2} },
                                    { {vm::vec3{1, 1, 1}, vm::vec3{2, 2, 2}, vm::vec3{0, 0, 1}}, vm::vec3{3, 3, 3} },
                                    { {vm::vec3{1, 1, 1}, vm::vec3{3, 3, 3}, vm::vec3{0, 0, 1}}, vm::vec3{4, 4, 4} },
                                });

                                AND_THEN("The other drag config was used") {
                                    CHECK(initialGetHandlePositionArguments.size() == 1);
                                    CHECK(otherGetHandlePositionArguments.size() == 2);
                                }
                            }
                        }
                    }
                }
            }
        }

        TEST_CASE("makeLineHandlePicker") {
            using T = std::tuple<vm::line3, vm::vec3, vm::ray3, vm::vec3>;

            const auto 
            [line,                                            handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
            {vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{ 0,  0,  0}, vm::ray3{vm::vec3{0, -1, 0}, vm::vec3{0, 1, 0}}, vm::vec3{0, 0, 0}},
            {vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{-1, -1, -1}, vm::ray3{vm::vec3{1, -1, 1}, vm::vec3{0, 1, 0}}, vm::vec3{0, 0, 0}}, // hitPoint is at {1 1 1}
            {vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{-1, -1, -1}, vm::ray3{vm::vec3{1, -1, 2}, vm::vec3{0, 1, 0}}, vm::vec3{0, 0, 1}}, // hitPoint is at {1 1 1}
            }));

            CAPTURE(line, handleOffset, pickRay);

            const auto camera = Renderer::OrthographicCamera{};
            auto inputState = InputState{};
            inputState.setPickRequest(PickRequest{pickRay, camera});

            CHECK(makeLineHandlePicker(line, handleOffset)(inputState) == expectedHandlePosition);
        }

        TEST_CASE("makePlaneHandlePicker") {
            using T = std::tuple<vm::plane3, vm::vec3, vm::ray3, vm::vec3>;

            const auto 
            [plane,                                            handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
            {vm::plane3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{ 0,  0,  0}, vm::ray3{vm::vec3{0, 0, 1}, vm::vec3{0, 0, -1}}, vm::vec3{0, 0, 0}},
            {vm::plane3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{-1, -1, -1}, vm::ray3{vm::vec3{1, 1, 1}, vm::vec3{0, 0, -1}}, vm::vec3{0, 0, 0}}, // hitPoint is at {1 1 1}
            {vm::plane3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{-1, -1, -1}, vm::ray3{vm::vec3{1, 2, 1}, vm::vec3{0, 0, -1}}, vm::vec3{0, 1, 0}}, // hitPoint is at {1 1 1}
            }));

            CAPTURE(plane, handleOffset, pickRay);

            const auto camera = Renderer::OrthographicCamera{};
            auto inputState = InputState{};
            inputState.setPickRequest(PickRequest{pickRay, camera});

            CHECK(makePlaneHandlePicker(plane, handleOffset)(inputState) == expectedHandlePosition);
        }

        TEST_CASE("makeCircleHandlePicker") {
            using T = std::tuple<vm::vec3, vm::vec3, FloatType, vm::vec3, vm::ray3, vm::vec3>;

            const auto 
            [center,            normal,            radius, handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}, 10.0,   vm::vec3{ 0,  0,  0}, vm::ray3{vm::vec3{5, 0, 1}, vm::vec3{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3{1, 0, 0})},
            {vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}, 10.0,   vm::vec3{ 0,  0,  1}, vm::ray3{vm::vec3{5, 0, 1}, vm::vec3{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3{1, 0, 0})},
            {vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}, 10.0,   vm::vec3{ 0,  0,  0}, vm::ray3{vm::vec3{5, 5, 1}, vm::vec3{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3{1, 1, 0})},
            {vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}, 10.0,   vm::vec3{ 1,  1,  1}, vm::ray3{vm::vec3{5, 5, 1}, vm::vec3{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3{1, 1, 0})},
            }));

            CAPTURE(center, normal, radius, handleOffset, pickRay);

            const auto camera = Renderer::OrthographicCamera{};
            auto inputState = InputState{};
            inputState.setPickRequest(PickRequest{pickRay, camera});

            CHECK(makeCircleHandlePicker(center, normal, radius, handleOffset)(inputState) == vm::approx{expectedHandlePosition});
        }

        TEST_CASE("makeSurfaceHandlePicker") {
            using namespace Model::HitFilters;

            static const auto HitType = Model::HitType::freeType();
            static const auto OtherHitType = Model::HitType::freeType();
            static const auto BothTypes = HitType | OtherHitType;

            const auto hit = Model::Hit{HitType, 10.0, vm::vec3{0, 0, 10}, size_t{1}};
            const auto otherHit = Model::Hit{OtherHitType, 12.0, vm::vec3{0, 0, 12}, size_t{2}};

            using T = std::tuple<Model::HitFilter, vm::vec3, vm::ray3, vm::vec3>;

            const auto
            [hitFilter,          handleOffset,      pickRay,                                          expectedHandlePosition] = GENERATE_REF(values<T>({
            {type(HitType),      vm::vec3{0, 0, 0}, vm::ray3{vm::vec3{0, 0, 20}, vm::vec3{0, 0, -1}}, vm::vec3{hit.hitPoint()}},
            {type(OtherHitType), vm::vec3{0, 0, 0}, vm::ray3{vm::vec3{0, 0, 20}, vm::vec3{0, 0, -1}}, vm::vec3{otherHit.hitPoint()}},
            {type(BothTypes),    vm::vec3{0, 0, 0}, vm::ray3{vm::vec3{0, 0, 20}, vm::vec3{0, 0, -1}}, vm::vec3{hit.hitPoint()}},
            {type(HitType),      vm::vec3{1, 1, 1}, vm::ray3{vm::vec3{0, 0, 20}, vm::vec3{0, 0, -1}}, vm::vec3{hit.hitPoint() + vm::vec3{1, 1, 1}}},
            }));

            CAPTURE(handleOffset, pickRay);

            const auto camera = Renderer::OrthographicCamera{};
            auto inputState = InputState{};
            inputState.setPickRequest(PickRequest{pickRay, camera});

            auto pickResult = Model::PickResult{};
            pickResult.addHit(hit);
            pickResult.addHit(otherHit);
            inputState.setPickResult(std::move(pickResult));

            CHECK(makeSurfaceHandlePicker(hitFilter, handleOffset)(inputState) == expectedHandlePosition);
        }

        TEST_CASE("makeIdentityHandleSnapper") {
            using T = std::tuple<vm::vec3, vm::vec3>;

            const auto 
            [proposedHandlePosition, expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{0, 0, 0}, vm::vec3{0, 0, 0}},
            {vm::vec3{1, 2, 3}, vm::vec3{1, 2, 3}},
            }));

            CAPTURE(proposedHandlePosition);

            CHECK(makeIdentityHandleSnapper()(InputState{}, DragState{}, proposedHandlePosition) == expectedHandlePosition);
        }

        TEST_CASE("makeRelativeHandleSnapper") {
            using T = std::tuple<vm::vec3, vm::vec3, int, vm::vec3>;

            const auto
            [initialHandlePosition, proposedHandlePosition, gridSize, expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{3, 1, 2},     vm::vec3{3, 1, 2},      4,        vm::vec3{3, 1, 2}},
            {vm::vec3{3, 1, 2},     vm::vec3{7, 1, 2},      4,        vm::vec3{3, 1, 2}},
            {vm::vec3{3, 1, 2},     vm::vec3{8, 1, 2},      3,        vm::vec3{11, 1, 2}},
            {vm::vec3{3, 1, 2},     vm::vec3{10, 1, 2},     4,        vm::vec3{3, 1, 2}},
            {vm::vec3{3, 1, 2},     vm::vec3{11, 1, 2},     4,        vm::vec3{19, 1, 2}},
            {vm::vec3{3, 1, 2},     vm::vec3{33, 1, 2},     4,        vm::vec3{35, 1, 2}},
            }));

            CAPTURE(initialHandlePosition, proposedHandlePosition, gridSize);

            const auto grid = Grid{gridSize};
            CHECK(makeRelativeHandleSnapper(grid)(InputState{}, DragState{initialHandlePosition, vm::vec3{}, vm::vec3{}}, proposedHandlePosition) == expectedHandlePosition);
        }

        TEST_CASE("makeAbsoluteHandleSnapper") {
            using T = std::tuple<vm::vec3, int, vm::vec3>;

            const auto
            [proposedHandlePosition, gridSize, expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{0, 0, 0},      4,        vm::vec3{0, 0, 0}},
            {vm::vec3{4, 3, 2},      4,        vm::vec3{0, 0, 0}},
            {vm::vec3{4, 3, 22},     3,        vm::vec3{8, 0, 24}},
            {vm::vec3{7, 0, 0},      4,        vm::vec3{0, 0, 0}},
            {vm::vec3{8, 17, 31},    4,        vm::vec3{16, 16, 32}},
            }));

            CAPTURE(proposedHandlePosition, gridSize);

            const auto grid = Grid{gridSize};
            CHECK(makeAbsoluteHandleSnapper(grid)(InputState{}, DragState{vm::vec3{}, vm::vec3{}, vm::vec3{}}, proposedHandlePosition) == expectedHandlePosition);
        }

        TEST_CASE("makeRelativeLineHandleSnapper") {
            using T = std::tuple<vm::vec3, vm::vec3, int, vm::line3, vm::vec3>;

            const auto
            [initialHandlePosition, proposedHandlePosition, gridSize, line,                                            expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{0, 0, 0},     vm::vec3{0, 0, 0},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{0, 0, 0},     vm::vec3{0, 0, 7},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{0, 0, 0},     vm::vec3{2, 9, 7},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{0, 0, 0},     vm::vec3{2, 9, 8},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 16}},
            {vm::vec3{0, 0, 1},     vm::vec3{2, 9, 8},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 1}},
            {vm::vec3{0, 0, 1},     vm::vec3{2, 9, 9},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 17}},
            {vm::vec3{22, 9, 1},    vm::vec3{2, 9, 9},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 17}},
            }));

            CAPTURE(initialHandlePosition, proposedHandlePosition, gridSize, line);

            const auto grid = Grid{gridSize};
            CHECK(makeRelativeLineHandleSnapper(grid, line)(InputState{}, DragState{initialHandlePosition, vm::vec3{}, vm::vec3{}}, proposedHandlePosition) == expectedHandlePosition);
        }

        TEST_CASE("makeAbsoluteLineHandleSnapper") {
            using T = std::tuple<vm::vec3, int, vm::line3, vm::vec3>;

            const auto
            [proposedHandlePosition, gridSize, line,                                            expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{0, 0, 0},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{0, 0, 7},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{0, 0, 7},      3,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 8}},
            {vm::vec3{2, 9, 7},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 0}},
            {vm::vec3{2, 9, 9},      4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 16}},
            {vm::vec3{2, 9, 31},     4,        vm::line3{vm::vec3{0, 0, 0}, vm::vec3{0, 0, 1}}, vm::vec3{0, 0, 32}},
            }));

            CAPTURE(proposedHandlePosition, gridSize, line);

            const auto grid = Grid{gridSize};
            CHECK(makeAbsoluteLineHandleSnapper(grid, line)(InputState{}, DragState{vm::vec3{}, vm::vec3{}, vm::vec3{}}, proposedHandlePosition) == expectedHandlePosition);
        }

        TEST_CASE("makeCircleHandleSnapper") {
            using T = std::tuple<vm::vec3, vm::vec3, FloatType, vm::vec3>;

            const auto
            [initialHandlePosition, proposedHandlePosition, snapAngle, expectedHandlePosition] = GENERATE(values<T>({
            {vm::vec3{1, 0, 0},     vm::vec3{1, 0, 0},      15.0,      vm::normalize(vm::vec3{1, 0, 0})},
            {vm::vec3{1, 0, 0},     vm::vec3{1, 1, 0},      15.0,      vm::normalize(vm::vec3{1, 1, 0})},
            {vm::vec3{1, 0, 0},     vm::vec3{1, 2, 0},      15.0,      vm::normalize(vm::vec3{0.5, 0.866025, 0})},
            {vm::vec3{1, 0, 0},     vm::vec3{1, 1, 0},      45.0,      vm::normalize(vm::vec3{1, 1, 0})},
            }));

            CAPTURE(initialHandlePosition, proposedHandlePosition, snapAngle);

            const auto grid = Grid{4};
            const auto center = vm::vec3{0, 0, 0};
            const auto normal = vm::vec3{0, 0, 1};
            const auto radius = 10.0;
            CHECK(makeCircleHandleSnapper(grid, vm::to_radians(snapAngle), center, normal, radius)(InputState{}, DragState{initialHandlePosition, vm::vec3{0, 0, 0}, vm::vec3{0, 0, 0}}, proposedHandlePosition) == vm::approx{radius * expectedHandlePosition});
        }
    }
}
