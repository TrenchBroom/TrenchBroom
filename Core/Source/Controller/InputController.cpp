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
#include "Controller/MoveFaceTool.h"
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
            if (m_currentEvent.pickResults != NULL)
                delete m_currentEvent.pickResults;
            
            Model::Picker& picker = m_editor.map().picker();
            Camera& camera = m_editor.camera();
            m_currentEvent.ray = camera.pickRay(m_currentEvent.mouseX, m_currentEvent.mouseY);
            m_currentEvent.pickResults = picker.pick(m_currentEvent.ray, m_editor.filter());

            for (unsigned int i = 0; i < m_receiverChain.size(); i++)
                m_receiverChain[i]->updateHits(m_currentEvent);
        }

        void InputController::updateMousePos(float x, float y) {
            m_currentEvent.deltaX = x - m_currentEvent.mouseX;
            m_currentEvent.deltaY = y - m_currentEvent.mouseY;
            m_currentEvent.mouseX = x;
            m_currentEvent.mouseY = y;
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
        
        bool InputController::modalToolActive(const ToolPtr& tool) {
            if (m_modalReceiverIndex == -1)
                return false;
            return m_receiverChain[m_modalReceiverIndex] == tool;
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
            
            m_dragButton = Tool::TB_MB_NONE;
            m_dragScrollReceiver = ToolPtr();

            m_moveVertexTool = ToolPtr(new MoveVertexTool(m_editor));
            m_moveEdgeTool = ToolPtr(new MoveEdgeTool(m_editor));
            m_moveFaceTool = ToolPtr(new MoveFaceTool(m_editor));
            
            DragTextureTargetTool* dragTextureTargetTool = new DragTextureTargetTool(m_editor);
            DragEntityTargetTool* dragEntityTargetTool = new DragEntityTargetTool(m_editor);
            
            m_dragTargetTools["Texture"] = DragTargetToolPtr(dragTextureTargetTool);
            m_dragTargetTools["Entity"] = DragTargetToolPtr(dragEntityTargetTool);
            
            m_currentEvent.mouseX = 0;
            m_currentEvent.mouseY = 0;
            m_currentEvent.deltaX = 0;
            m_currentEvent.deltaY = 0;
        }
        
        InputController::~InputController() {
            if (m_currentEvent.pickResults != NULL)
                delete m_currentEvent.pickResults;
        }
        
        void InputController::toggleMoveVertexTool() {
            toggleModalTool(m_moveVertexTool, 1);
        }

        void InputController::toggleMoveEdgeTool() {
            toggleModalTool(m_moveEdgeTool, 1);
        }
        
        void InputController::toggleMoveFaceTool() {
            toggleModalTool(m_moveFaceTool, 1);
        }
        
        bool InputController::moveVertexToolActive() {
            return modalToolActive(m_moveVertexTool);
        }
        
        bool InputController::moveEdgeToolActive() {
            return modalToolActive(m_moveEdgeTool);
        }
        
        bool InputController::moveFaceToolActive() {
            return modalToolActive(m_moveFaceTool);
        }
        
        
        bool InputController::key(wchar_t c) {
            return false;
        }

        void InputController::modifierKeyDown(Tool::EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys |= modifierKey;
            updateHits();
            
            ToolList chain = m_receiverChain;
            for (unsigned int i = 0; i < chain.size(); i++)
                chain[i]->modifierKeyChanged(m_currentEvent);
        }
        
        void InputController::modifierKeyUp(Tool::EModifierKeys modifierKey) {
            m_currentEvent.modifierKeys &= ~modifierKey;
            updateHits();
            
            ToolList chain = m_receiverChain;
            for (unsigned int i = 0; i < chain.size(); i++)
                chain[i]->modifierKeyChanged(m_currentEvent);
        }
        
        bool InputController::mouseDown(Tool::EMouseButton mouseButton, float x, float y) {
            m_currentEvent.mouseButton = mouseButton;
            updateMousePos(x, y);
            updateHits();
            
            ToolList chain = m_receiverChain;
            for (unsigned int i = 0; i < chain.size(); i++) {
                if (chain[i]->mouseDown(m_currentEvent)) {
                    m_mouseDownReceiver = chain[i];
                    return true;
                }
            }
            
            return false;
        }
        
        bool InputController::mouseUp(Tool::EMouseButton mouseButton, float x, float y) {
            m_currentEvent.mouseButton = mouseButton;
            updateMousePos(x, y);
            updateHits();
            
            bool handled = false;
            if (m_currentEvent.mouseButton == m_dragButton) {
                if (m_dragScrollReceiver.get() != NULL)
                    m_dragScrollReceiver->endDrag(m_currentEvent);
                if (m_mouseDownReceiver.get() != NULL && m_mouseDownReceiver != m_dragScrollReceiver)
                    m_mouseDownReceiver->mouseUp(m_currentEvent);
                m_dragScrollReceiver = ToolPtr();
                m_dragButton = Tool::TB_MB_NONE;
                handled = true;
            } else {
                ToolList chain = m_receiverChain;
                for (unsigned int i = 0; i < chain.size() && !handled; i++)
                    if (chain[i]->mouseUp(m_currentEvent))
                        handled = true;
            }
            
            m_mouseDownReceiver = ToolPtr();
            m_currentEvent.mouseButton = Tool::TB_MB_NONE;
            return handled;
        }
        
        void InputController::mouseMoved(float x, float y) {
            if (m_currentEvent.mouseButton != Tool::TB_MB_NONE && m_dragButton == Tool::TB_MB_NONE) {
                m_dragButton = m_currentEvent.mouseButton;
                ToolList chain = m_receiverChain;
                for (unsigned int i = 0; i < chain.size(); i++) {
                    if (chain[i]->beginDrag(m_currentEvent)) {
                        m_dragScrollReceiver = chain[i];
                        break;
                    }
                }
            }

            updateMousePos(x, y);
            updateHits();
            
            if (m_dragButton != Tool::TB_MB_NONE && m_dragScrollReceiver.get() != NULL) {
                if (!m_dragScrollReceiver->drag(m_currentEvent)) {
                    m_dragScrollReceiver = ToolPtr();
                    m_mouseDownReceiver = ToolPtr();
                }
            }
            
            if (m_dragButton == Tool::TB_MB_NONE || m_dragScrollReceiver.get() == NULL) {
                ToolList chain = m_receiverChain;
                for (unsigned int i = 0; i < chain.size(); i++)
                    chain[i]->mouseMoved(m_currentEvent);
            }
        }
        
        void InputController::scrolled(float dx, float dy) {
            m_currentEvent.scrollX = dx;
            m_currentEvent.scrollY = dy;
            updateHits();
            
            if (m_dragScrollReceiver != NULL) {
                m_dragScrollReceiver->scrolled(m_currentEvent);
            } else {
                ToolList chain = m_receiverChain;
                for (unsigned int i = 0; i < chain.size(); i++)
                    if (chain[i]->scrolled(m_currentEvent))
                        break;
            }
        }

        bool InputController::dragEnter(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            updateHits();

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
            updateHits();

            for (DragTargetToolMap::iterator it = m_dragTargetTools.begin(); it != m_dragTargetTools.end(); ++it)
                it->second->deactivate(m_currentDragInfo);
        }
        
        bool InputController::dragMove(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            updateHits();

            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return true;
            
            return it->second->move(m_currentDragInfo);
        }

        bool InputController::acceptDrag(const std::string& name, void* payload) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            updateHits();
            
            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return false;
            
            return it->second->accepts(m_currentDragInfo);
        }
        
        bool InputController::handleDrop(const std::string& name, void* payload, float x, float y) {
            m_currentDragInfo.name = name;
            m_currentDragInfo.payload = payload;
            updateHits();
            
            DragTargetToolMap::iterator it = m_dragTargetTools.find(name);
            if (it == m_dragTargetTools.end())
                return false;
            
            return it->second->drop(m_currentDragInfo);
        }
    }
}
