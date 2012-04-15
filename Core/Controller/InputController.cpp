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
#include "Controller/Camera.h"
#include "Controller/CameraTool.h"
#include "Controller/Editor.h"
#include "Controller/SelectionTool.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateHits() {
            if (m_currentEvent.hits != NULL)
                delete m_currentEvent.hits;
            
            Model::Picker& picker = m_editor.map().picker();
            Camera& camera = m_editor.camera();
            Ray pickRay = camera.pickRay(m_currentEvent.mouseX, m_currentEvent.mouseY);
            m_currentEvent.hits = picker.pick(pickRay, m_editor.filter());
        }

        InputController::InputController(Editor& editor) : m_editor(editor) {
            m_cameraTool = new CameraTool(m_editor);
            m_selectionTool = new SelectionTool(m_editor);
            m_receiverChain.push_back(m_cameraTool);
            m_receiverChain.push_back(m_selectionTool);
            m_dragStatus = MS_NONE;
            m_dragScrollReceiver = NULL;
        }
        
        InputController::~InputController() {
            delete m_cameraTool;
            delete m_selectionTool;
        }
        
        void InputController::modifierKeyDown(EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys |= modifierKey;
        }
        
        void InputController::modifierKeyUp(EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys &= ~modifierKey;
        }
        
        void InputController::mouseDown(EMouseButton mouseButton) {
            m_currentEvent.mouseButton = mouseButton;
            
            if (m_currentEvent.mouseButton == MB_LEFT) {
                for (int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->leftMouseDown(m_currentEvent))
                        break;
            } else if (m_currentEvent.mouseButton == MB_RIGHT) {
                for (int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->rightMouseDown(m_currentEvent))
                        break;
            }
        }
        
        void InputController::mouseUp(EMouseButton mouseButton) {
            m_currentEvent.mouseButton = mouseButton;
            
            if (m_currentEvent.mouseButton == MB_LEFT) {
                if (m_dragStatus == MS_LEFT) {
                    if (m_dragScrollReceiver != NULL)
                        m_dragScrollReceiver->endLeftDrag(m_currentEvent);
                    m_dragScrollReceiver = NULL;
                    m_dragStatus = MS_NONE;
                } else {
                    for (int i = 0; i < m_receiverChain.size(); i++)
                        if (m_receiverChain[i]->leftMouseUp(m_currentEvent))
                            break;
                }
            } else if (m_currentEvent.mouseButton == MB_RIGHT) {
                if (m_dragStatus == MS_RIGHT) {
                    if (m_dragScrollReceiver != NULL)
                        m_dragScrollReceiver->endRightDrag(m_currentEvent);
                    m_dragScrollReceiver = NULL;
                    m_dragStatus = MS_NONE;
                } else {
                    for (int i = 0; i < m_receiverChain.size(); i++)
                        if (m_receiverChain[i]->rightMouseUp(m_currentEvent))
                            break;
                }
            }
            
            m_currentEvent.mouseButton = MB_NONE;
        }
        
        void InputController::mouseMoved(float x, float y, float dx, float dy) {
            m_currentEvent.mouseX = x;
            m_currentEvent.mouseY = y;
            m_currentEvent.deltaX = dx;
            m_currentEvent.deltaY = dy;
            updateHits();
            
            if (m_currentEvent.mouseButton != MB_NONE && m_dragStatus == MS_NONE) {
                if (m_currentEvent.mouseButton == MB_LEFT) {
                    m_dragStatus = MS_LEFT;
                    for (int i = 0; i < m_receiverChain.size(); i++) {
                        if (m_receiverChain[i]->beginLeftDrag(m_currentEvent)) {
                            m_dragScrollReceiver = m_receiverChain[i];
                            break;
                        }
                    }
                } else if (m_currentEvent.mouseButton == MB_RIGHT) {
                    m_dragStatus = MS_RIGHT;
                    for (int i = 0; i < m_receiverChain.size(); i++) {
                        if (m_receiverChain[i]->beginRightDrag(m_currentEvent)) {
                            m_dragScrollReceiver = m_receiverChain[i];
                            break;
                        }
                    }
                }
            }
            
            if (m_dragStatus == MS_LEFT && m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->leftDrag(m_currentEvent);
            } else if (m_dragStatus == MS_RIGHT && m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->rightDrag(m_currentEvent);
            } else {
                for (int i = 0; i < m_receiverChain.size(); i++)
                    m_receiverChain[i]->mouseMoved(m_currentEvent);
            }
        }
        
        void InputController::scrolled(float dx, float dy) {
            m_currentEvent.scrollX = dx;
            m_currentEvent.scrollY = dy;
            
            if (m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->scrolled(m_currentEvent);
            } else {
                for (int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->scrolled(m_currentEvent))
                        break;
            }
        }
    }
}
