/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__InputState__
#define __TrenchBroom__InputState__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        typedef unsigned int ModifierKeyState;
        namespace ModifierKeys {
            static const ModifierKeyState MKNone      = 0;
            static const ModifierKeyState MKShift     = 1 << 0;
            static const ModifierKeyState MKCtrlCmd   = 1 << 1; // Cmd on Mac, Ctrl on other systems
            static const ModifierKeyState MKAlt       = 1 << 2;
        }
        
        typedef unsigned int MouseButtonState;
        namespace MouseButtons {
            static const MouseButtonState MBNone      = 0;
            static const MouseButtonState MBLeft      = 1 << 0;
            static const MouseButtonState MBRight     = 1 << 1;
            static const MouseButtonState MBMiddle    = 1 << 2;
        }
        
        class Grid;
        
        class InputState {
        private:
            ModifierKeyState m_modifierKeys;
            MouseButtonState m_mouseButtons;
            int m_mouseX;
            int m_mouseY;
            int m_mouseDX;
            int m_mouseDY;
            float m_scrollX;
            float m_scrollY;
            
            Ray3 m_pickRay;
            mutable Model::PickResult m_pickResult;

            const Renderer::Camera& m_camera;
        public:
            InputState(const Renderer::Camera& camera);
            InputState(const Renderer::Camera& camera, const int mouseX, const int mouseY);
            virtual ~InputState();
            
            virtual ModifierKeyState modifierKeys() const;
            bool modifierKeysDown(const ModifierKeyState keys) const;
            bool modifierKeysPressed(const ModifierKeyState keys) const;
            
            MouseButtonState mouseButtons() const;
            bool mouseButtonsDown(const MouseButtonState buttons) const;
            bool mouseButtonsPressed(const MouseButtonState buttons) const;
            
            int mouseX() const;
            int mouseY() const;
            int mouseDX() const;
            int mouseDY() const;
            
            float scrollX() const;
            float scrollY() const;

            void setModifierKeys(const ModifierKeyState keys);
            void clearModifierKeys();
            void mouseDown(const MouseButtonState button);
            void mouseUp(const MouseButtonState button);
            void clearMouseButtons();
            void mouseMove(const int mouseX, const int mouseY);
            void scroll(const float scrollX, const float scrollY);

            const Ray3& pickRay() const;
            void setPickRay(const Ray3& pickRay);
            Model::PickResult& pickResult() const;
            void setPickResult(Model::PickResult& pickResult);

            const Renderer::Camera& camera() const;
        };
    }
}

#endif /* defined(__TrenchBroom__InputState__) */
