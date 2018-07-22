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

#include <gtest/gtest.h>

#include "Model/EditorContext.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContextTest : public ::testing::Test {
        protected:
            World* world;
            EditorContext context;

            void SetUp() override {
                world = new World(MapFormat::Standard, nullptr, BBox3d(8192.0));
            }

            void TearDown() override {
                context.reset();
                delete world;
                world = nullptr;
            }

            void assertVisible(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.visible(node));
            }

            void assertEditable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.editable(node));
            }

            void assertPickable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.pickable(node));
            }

            void assertSelectable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.selectable(node));
            }
        private:
            void setState(Node* node, const VisibilityState visibilityState, const LockState lockState) {
                node->setVisiblityState(visibilityState);
                node->setLockState(lockState);
            }
        };

        TEST_F(EditorContextTest, testWorldVisible) {
            assertVisible(true, world, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, world, Visibility_Shown, Lock_Locked);
            assertVisible(false, world, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldEditable) {
            assertEditable(true, world, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, world, Visibility_Shown, Lock_Locked);
            assertEditable(true, world, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldPickable) {
            assertPickable(false, world, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, world, Visibility_Shown, Lock_Locked);
            assertPickable(false, world, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldSelectable) {
            assertSelectable(false, world, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, world, Visibility_Shown, Lock_Locked);
            assertSelectable(false, world, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, world, Visibility_Hidden, Lock_Locked);
        }
    }
}
