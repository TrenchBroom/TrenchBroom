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

#include "InputController.h"

#include "Controller/CameraTool.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateHits() {
            if (m_currentEvent.pickResult != NULL)
                delete m_currentEvent.pickResult;
            
            m_currentEvent.camera = &m_camera;
            m_currentEvent.ray = m_camera.pickRay(m_currentEvent.mouseX, m_currentEvent.mouseY);
            m_currentEvent.pickResult = m_picker.pick(m_currentEvent.ray);
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->updateHits(m_currentEvent);
        }

        void InputController::updateMousePos(float x, float y) {
            m_currentEvent.deltaX = x - m_currentEvent.mouseX;
            m_currentEvent.deltaY = y - m_currentEvent.mouseY;
            m_currentEvent.mouseX = x;
            m_currentEvent.mouseY = y;
        }

        InputController::InputController(Model::MapDocument& document, Renderer::Camera& camera, wxWindow& control) :
        m_dragReceiver(NULL),
        m_mouseUpReceiver(NULL),
        m_modalReceiverIndex(-1),
        m_picker(document.Picker()),
        m_camera(camera) {
            CameraTool* cameraTool = new CameraTool(control);
            m_receivers.push_back(cameraTool);
        }
        
        InputController::~InputController() {
        }

        void InputController::modifierKeyDown(ModifierKeyState modifierKey) {
            m_currentEvent.modifierKeys |= modifierKey;
            updateHits();
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->modifierKeyChanged(m_currentEvent);
        }
        
        void InputController::modifierKeyUp(ModifierKeyState modifierKey) {
            m_currentEvent.modifierKeys &= ~modifierKey;
            updateHits();
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->modifierKeyChanged(m_currentEvent);
        }
        
        bool InputController::mouseDown(MouseButtonState mouseButton, float x, float y) {
            m_currentEvent.mouseButtons |= mouseButton;
            updateMousePos(x, y);
            updateHits();
            
            for (unsigned int i = 0; i < m_receivers.size(); i++) {
                if (m_receivers[i]->mouseDown(m_currentEvent)) {
                    m_mouseUpReceiver = m_receivers[i];
                    return true;
                }
            }
            
            return false;
        }
        
        bool InputController::mouseUp(MouseButtonState mouseButton, float x, float y) {
            m_currentEvent.mouseButtons &= ~mouseButton;
            updateMousePos(x, y);
            updateHits();
            
            bool handled = false;
            if (m_dragButtons != MouseButtons::None) {
                if (m_dragReceiver != NULL)
                    m_dragReceiver->endDrag(m_currentEvent);
                if (m_mouseUpReceiver != NULL && m_mouseUpReceiver != m_dragReceiver)
                    m_mouseUpReceiver->mouseUp(m_currentEvent);
                m_dragReceiver = NULL;
                m_dragButtons = MouseButtons::None;
                handled = true;
            } else {
                for (unsigned int i = 0; i < m_receivers.size() && !handled; i++)
                    if (m_receivers[i]->mouseUp(m_currentEvent))
                        handled = true;
            }
            
            m_mouseUpReceiver = NULL;
            m_currentEvent.mouseButtons = MouseButtons::None;
            return handled;
        }
        
        void InputController::mouseMoved(float x, float y) {
            if (m_currentEvent.mouseButtons != MouseButtons::None && m_dragButtons == MouseButtons::None) {
                m_dragButtons = m_currentEvent.mouseButtons;
                for (unsigned int i = 0; i < m_receivers.size(); i++) {
                    if (m_receivers[i]->beginDrag(m_currentEvent)) {
                        m_dragReceiver = m_receivers[i];
                        break;
                    }
                }
            }
            
            updateMousePos(x, y);
            updateHits();
            
            if (m_dragButtons != MouseButtons::None && m_dragReceiver != NULL) {
                if (!m_dragReceiver->drag(m_currentEvent)) {
                    m_dragReceiver = NULL;
                    m_mouseUpReceiver = NULL;
                }
            }
            
            if (m_dragButtons == MouseButtons::None || m_dragReceiver == NULL) {
                for (unsigned int i = 0; i < m_receivers.size(); i++)
                    m_receivers[i]->mouseMoved(m_currentEvent);
            }
        }
        
        void InputController::scrolled(float dx, float dy) {
            m_currentEvent.scrollX = dx;
            m_currentEvent.scrollY = dy;
            updateHits();
            
            if (m_dragReceiver != NULL) {
                m_dragReceiver->scrolled(m_currentEvent);
            } else {
                for (unsigned int i = 0; i < m_receivers.size(); i++)
                    if (m_receivers[i]->scrolled(m_currentEvent))
                        break;
            }
        }
    }
}