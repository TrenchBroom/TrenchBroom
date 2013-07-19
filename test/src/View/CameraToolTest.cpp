/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "View/CameraTool.h"
#include "View/InputState.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        class TestInputState : public InputState {
        private:
            ModifierKeyState m_modifierKeys;
        public:
            TestInputState(const int mouseX, const int mouseY) :
            InputState(mouseX, mouseY),
            m_modifierKeys(ModifierKeys::MKNone) {}
            
            ModifierKeyState modifierKeys() const {
                return m_modifierKeys;
            }
            
            void modifierKeyDown(const ModifierKeyState key) {
                m_modifierKeys |= key;
            }
            
            void modifierKeyUp(const ModifierKeyState key) {
                m_modifierKeys &= ~key;
            }
        };
        
        TEST(CameraToolTest, cameraLook) {
            Renderer::Camera camera(90.0f, 1.0f, 100.0f, Renderer::Camera::Viewport(0, 0, 1024, 768),
                                    Vec3f::Null, Vec3f::PosX, Vec3f::PosZ);
            TestInputState inputState(0, 0);
            CameraTool tool(NULL, camera);

            inputState.mouseDown(MouseButtons::MBRight);
            ASSERT_FALSE(tool.startMouseDrag(inputState) == NULL);
            
            inputState.mouseMove(10, 0);
            ASSERT_TRUE(tool.mouseDrag(inputState));
            
            inputState.mouseUp(MouseButtons::MBRight);
            tool.endMouseDrag(inputState);
            
            ASSERT_VEC_EQ(Vec3f::Null, camera.position());
            ASSERT_VEC_NE(Vec3f::PosX, camera.direction());
            ASSERT_VEC_EQ(Vec3f::PosZ, camera.up());
        }
        
        TEST(CameraToolTest, cameraPan) {
            Renderer::Camera camera(90.0f, 1.0f, 100.0f, Renderer::Camera::Viewport(0, 0, 1024, 768),
                                    Vec3f::Null, Vec3f::PosX, Vec3f::PosZ);
            TestInputState inputState(0, 0);
            CameraTool tool(NULL, camera);
            
            inputState.mouseDown(MouseButtons::MBMiddle);
            ASSERT_FALSE(tool.startMouseDrag(inputState) == NULL);
            
            inputState.mouseMove(10, 10);
            ASSERT_TRUE(tool.mouseDrag(inputState));
            
            inputState.mouseUp(MouseButtons::MBMiddle);
            tool.endMouseDrag(inputState);
            
            ASSERT_VEC_NE(Vec3f::Null, camera.position());
            ASSERT_FLOAT_EQ(0.0f, camera.position().x());
            ASSERT_VEC_EQ(Vec3f::PosX, camera.direction());
            ASSERT_VEC_EQ(Vec3f::PosZ, camera.up());
        }
        
        TEST(CameraToolTest, cameraAltPan) {
            Renderer::Camera camera(90.0f, 1.0f, 100.0f, Renderer::Camera::Viewport(0, 0, 1024, 768),
                                    Vec3f::Null, Vec3f::PosX, Vec3f::PosZ);
            TestInputState inputState(0, 0);
            CameraTool tool(NULL, camera);
            
            SetTemporaryPreference<bool> setAltMove(Preferences::CameraEnableAltMove, true);
            inputState.mouseDown(MouseButtons::MBMiddle);
            inputState.modifierKeyDown(ModifierKeys::MKAlt);
            ASSERT_FALSE(tool.startMouseDrag(inputState) == NULL);
            
            inputState.mouseMove(10, 10);
            ASSERT_TRUE(tool.mouseDrag(inputState));
            
            inputState.mouseUp(MouseButtons::MBMiddle);
            tool.endMouseDrag(inputState);
            
            ASSERT_VEC_NE(Vec3f::Null, camera.position());
            ASSERT_FLOAT_EQ(0.0f, camera.position().z());
            ASSERT_VEC_EQ(Vec3f::PosX, camera.direction());
            ASSERT_VEC_EQ(Vec3f::PosZ, camera.up());
        }
    }
}
