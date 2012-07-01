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
#include "Controller/CreateBrushTool.h"
#include "Controller/DragEntityTargetTool.h"
#include "Controller/DragTextureTargetTool.h"
#include "Controller/Editor.h"
#include "Controller/MoveEdgeTool.h"
#include "Controller/MoveObjectTool.h"
#include "Controller/MoveVertexTool.h"
#include "Controller/SelectionTool.h"
#include "Controller/ResizeBrushTool.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateHits() {
            if (m_currentEvent.hits != NULL)
                delete m_currentEvent.hits;
            
            Model::Picker& picker = m_editor.map().picker();
            Camera& camera = m_editor.camera();
            m_currentEvent.ray = camera.pickRay(m_currentEvent.mouseX, m_currentEvent.mouseY);
            m_currentEvent.hits = picker.pick(m_currentEvent.ray, m_editor.filter());
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            
            Model::Selection& selection = m_editor.map().selection();
            const Model::BrushList& brushes = selection.brushes();
            for (unsigned int i = 0; i < brushes.size(); i++)
                brushes[i]->pickVertices(m_currentEvent.ray, prefs.vertexHandleSize(), *m_currentEvent.hits);
        }

        void InputController::toggleModalTool(const ToolPtr& tool, unsigned int index) {
            ToolPtr modalTool;
            if (m_modalReceiverIndex != -1) {
                modalTool = m_receiverChain[m_modalReceiverIndex];
                modalTool->deactivated(m_currentEvent);
                m_receiverChain.erase(m_receiverChain.begin() + m_modalReceiverIndex);
                m_modalReceiverIndex = -1;
            }
            
            if (tool != modalTool) {
                m_modalReceiverIndex = index;
                m_receiverChain.insert(m_receiverChain.begin() + m_modalReceiverIndex, tool);
                tool->activated(m_currentEvent);
            }
        }
        
        InputController::InputController(Editor& editor) : m_editor(editor), m_currentDragInfo(DragInfo(m_currentEvent)), m_modalReceiverIndex(-1) {
            ToolPtr cameraTool = ToolPtr(new CameraTool(m_editor));
            ToolPtr selectionTool = ToolPtr(new SelectionTool(m_editor));
            ToolPtr moveObjectTool = ToolPtr(new MoveObjectTool(m_editor));
            ToolPtr createBrushTool = ToolPtr(new CreateBrushTool(m_editor));
            ToolPtr resizeBrushTool = ToolPtr(new ResizeBrushTool(m_editor));
            
            m_receiverChain.push_back(cameraTool);
            m_receiverChain.push_back(resizeBrushTool);
            m_receiverChain.push_back(selectionTool);
            m_receiverChain.push_back(moveObjectTool);
            m_receiverChain.push_back(createBrushTool);
            
            m_dragStatus = TB_MS_NONE;
            m_dragScrollReceiver = ToolPtr();

            m_moveVertexTool = ToolPtr(new MoveVertexTool(m_editor));
            m_moveEdgeTool = ToolPtr(new MoveEdgeTool(m_editor));
            
            DragTextureTargetTool* dragTextureTargetTool = new DragTextureTargetTool(m_editor);
            DragEntityTargetTool* dragEntityTargetTool = new DragEntityTargetTool(m_editor);
            
            m_dragTargetTools["Texture"] = DragTargetToolPtr(dragTextureTargetTool);
            m_dragTargetTools["Entity"] = DragTargetToolPtr(dragEntityTargetTool);
        }
        
        InputController::~InputController() {
            if (m_currentEvent.hits != NULL)
                delete m_currentEvent.hits;
        }
        
        void InputController::toggleMoveVertexTool() {
            toggleModalTool(m_moveVertexTool, 1);
        }

        void InputController::toggleMoveEdgeTool() {
            toggleModalTool(m_moveEdgeTool, 1);
        }
        
        bool InputController::key(wchar_t c) {
            switch (c) {
                case L'e':
                    toggleMoveEdgeTool();
                    return true;
                case L'v':
                    toggleMoveVertexTool();
                    return true;
                default:
                    return false;
            }
        }

        void InputController::modifierKeyDown(EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys |= modifierKey;
        }
        
        void InputController::modifierKeyUp(EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys &= ~modifierKey;
        }
        
        void InputController::mouseDown(EMouseButton mouseButton) {
            m_currentEvent.mouseButton = mouseButton;
            
            if (m_currentEvent.mouseButton == TB_MB_LEFT) {
                for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->leftMouseDown(m_currentEvent))
                        break;
            } else if (m_currentEvent.mouseButton == TB_MB_RIGHT) {
                for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->rightMouseDown(m_currentEvent))
                        break;
            }
        }
        
        void InputController::mouseUp(EMouseButton mouseButton) {
            m_currentEvent.mouseButton = mouseButton;
            
            if (m_currentEvent.mouseButton == TB_MB_LEFT) {
                if (m_dragStatus == TB_MS_LEFT) {
                    if (m_dragScrollReceiver != NULL)
                        m_dragScrollReceiver->endLeftDrag(m_currentEvent);
                    m_dragScrollReceiver = ToolPtr();
                    m_dragStatus = TB_MS_NONE;
                } else {
                    for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                        if (m_receiverChain[i]->leftMouseUp(m_currentEvent))
                            break;
                }
            } else if (m_currentEvent.mouseButton == TB_MB_RIGHT) {
                if (m_dragStatus == TB_MS_RIGHT) {
                    if (m_dragScrollReceiver != NULL)
                        m_dragScrollReceiver->endRightDrag(m_currentEvent);
                    m_dragScrollReceiver = ToolPtr();
                    m_dragStatus = TB_MS_NONE;
                } else {
                    for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                        if (m_receiverChain[i]->rightMouseUp(m_currentEvent))
                            break;
                }
            }
            
            m_currentEvent.mouseButton = TB_MB_NONE;
        }
        
        void InputController::mouseMoved(float x, float y, float dx, float dy) {
            m_currentEvent.mouseX = x;
            m_currentEvent.mouseY = y;
            m_currentEvent.deltaX = dx;
            m_currentEvent.deltaY = dy;
            updateHits();
            
            if (m_currentEvent.mouseButton != TB_MB_NONE && m_dragStatus == TB_MS_NONE) {
                if (m_currentEvent.mouseButton == TB_MB_LEFT) {
                    m_dragStatus = TB_MS_LEFT;
                    for (unsigned int i = 0; i < m_receiverChain.size(); i++) {
                        if (m_receiverChain[i]->beginLeftDrag(m_currentEvent)) {
                            m_dragScrollReceiver = m_receiverChain[i];
                            break;
                        }
                    }
                } else if (m_currentEvent.mouseButton == TB_MB_RIGHT) {
                    m_dragStatus = TB_MS_RIGHT;
                    for (unsigned int i = 0; i < m_receiverChain.size(); i++) {
                        if (m_receiverChain[i]->beginRightDrag(m_currentEvent)) {
                            m_dragScrollReceiver = m_receiverChain[i];
                            break;
                        }
                    }
                }
            }
            
            if (m_dragStatus == TB_MS_LEFT && m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->leftDrag(m_currentEvent);
            } else if (m_dragStatus == TB_MS_RIGHT && m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->rightDrag(m_currentEvent);
            } else {
                for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                    m_receiverChain[i]->mouseMoved(m_currentEvent);
            }
        }
        
        void InputController::scrolled(float dx, float dy) {
            m_currentEvent.scrollX = dx;
            m_currentEvent.scrollY = dy;
            
            if (m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->scrolled(m_currentEvent);
            } else {
                for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                    if (m_receiverChain[i]->scrolled(m_currentEvent))
                        break;
            }
        }

        bool InputController::dragEnter(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;

            bool overlayVisible = true;
            for (DragTargetToolMap::iterator it = m_dragTargetTools.begin(); it != m_dragTargetTools.end(); ++it) {
                it->second->deactivate(m_currentDragInfo);
                if (it->first == name)
                    overlayVisible = it->second->activate(m_currentDragInfo);
            }
        
            return overlayVisible;
        }
        
        void InputController::dragLeave(const std::string& name, void* payload) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;

            for (DragTargetToolMap::iterator it = m_dragTargetTools.begin(); it != m_dragTargetTools.end(); ++it)
                it->second->deactivate(m_currentDragInfo);
        }
        
        bool InputController::dragMove(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;

            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return true;
            
            return it->second->move(m_currentDragInfo);
        }

        bool InputController::acceptDrag(const std::string& name, void* payload) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            
            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return false;
            
            return it->second->accepts(m_currentDragInfo);
        }
        
        bool InputController::handleDrop(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            
            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return false;
            
            return it->second->drop(m_currentDragInfo);
        }
    }
}
