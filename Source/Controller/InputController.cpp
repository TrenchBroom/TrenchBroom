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
#include "Controller/ClipTool.h"
#include "Controller/EntityDragTargetTool.h"
#include "Controller/MoveObjectsTool.h"
#include "Controller/RotateObjectsTool.h"
#include "Controller/SelectionTool.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Renderer/Camera.h"
#include "Renderer/InputControllerFeedbackFigure.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateHits() {
            if (m_currentEvent.pickResult != NULL)
                delete m_currentEvent.pickResult;
            
            if (!m_documentViewHolder.valid())
                return;
            
            Renderer::Camera& camera = m_documentViewHolder.view().camera();
            Model::Picker& picker = m_documentViewHolder.document().picker();
            
            m_currentEvent.camera = &camera;
            m_currentEvent.ray = camera.pickRay(m_currentEvent.mouseX, m_currentEvent.mouseY);
            m_currentEvent.pickResult = picker.pick(m_currentEvent.ray);
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->updateHits(m_currentEvent);
        }

        void InputController::updateFeedback() {
            bool updateViews = false;
            if (m_singleFeedbackProvider == NULL) {
                ToolList::iterator it, end;
                for (it = m_receivers.begin(), end = m_receivers.end(); it != end; ++it) {
                    Tool& tool = **it;
                    if (tool.suppressOtherFeedback(m_currentEvent)) {
                        m_singleFeedbackProvider = &tool;
                        updateViews = true;
                        break;
                    }
                }
            } else {
                if (!m_singleFeedbackProvider->suppressOtherFeedback(m_currentEvent)) {
                    m_singleFeedbackProvider = NULL;
                    updateViews = true;
                }
            }
            
            m_figureHolder->setSingleFeedbackProvider(m_singleFeedbackProvider);

            if (m_singleFeedbackProvider == NULL) {
                ToolList::iterator it, end;
                for (it = m_receivers.begin(), end = m_receivers.end(); it != end; ++it) {
                    Tool& tool = **it;
                    updateViews |= tool.updateFeedback(m_currentEvent);
                }
            } else {
                updateViews |= m_singleFeedbackProvider->updateFeedback(m_currentEvent);
            }
            
            if (updateViews)
                m_documentViewHolder.document().UpdateAllViews();
        }

        void InputController::updateMousePos(float x, float y) {
            m_currentEvent.deltaX = x - m_currentEvent.mouseX;
            m_currentEvent.deltaY = y - m_currentEvent.mouseY;
            m_currentEvent.mouseX = x;
            m_currentEvent.mouseY = y;
        }

        bool InputController::activateModalTool(Tool* modalTool) {
            if (deactivateModalTool()) {
                m_receivers.insert(m_receivers.begin() + ModalReceiverIndex, modalTool);
                modalTool->activated(m_currentEvent);
                m_modalToolActive = true;
                updateHits();
                updateFeedback();
                return true;
            }
            return false;
        }

        InputController::InputController(View::DocumentViewHolder& documentViewHolder) :
        m_documentViewHolder(documentViewHolder),
        m_dragReceiver(NULL),
        m_mouseUpReceiver(NULL),
        m_singleFeedbackProvider(NULL),
        m_modalToolActive(false),
        m_dragTargetReceiver(NULL),
        m_figureHolder(new Renderer::InputControllerFeedbackFigure()) {
            m_receivers.push_back(new CameraTool(m_documentViewHolder, *this));
            m_receivers.push_back(new MoveObjectsTool(m_documentViewHolder, *this));
            m_receivers.push_back(new RotateObjectsTool(m_documentViewHolder, *this));
            m_receivers.push_back(new SelectionTool(m_documentViewHolder, *this));
            m_dragTargetTools.push_back(new EntityDragTargetTool(m_documentViewHolder));

            m_clipTool = new ClipTool(documentViewHolder, *this);
            
            m_documentViewHolder.view().renderer().addFigure(m_figureHolder);
        }
        
        InputController::~InputController() {
            if (m_figureHolder != NULL && m_documentViewHolder.valid())
                m_documentViewHolder.view().renderer().deleteFigure(m_figureHolder);
            while (!m_receivers.empty()) delete m_receivers.back(), m_receivers.pop_back();
            while (!m_dragTargetTools.empty()) delete m_dragTargetTools.back(), m_dragTargetTools.pop_back();
        }

        void InputController::modifierKeyDown(ModifierKeyState modifierKey) {
            if (!m_documentViewHolder.valid())
                return;
            
            updateHits();
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->modifierKeyChanged(m_currentEvent);
            updateFeedback();
        }
        
        void InputController::modifierKeyUp(ModifierKeyState modifierKey) {
            if (!m_documentViewHolder.valid())
                return;
            
            updateHits();
            
            for (unsigned int i = 0; i < m_receivers.size(); i++)
                m_receivers[i]->modifierKeyChanged(m_currentEvent);
            updateFeedback();
        }
        
        bool InputController::mouseDown(MouseButtonState mouseButton, float x, float y) {
            if (!m_documentViewHolder.valid())
                return false;
            
            m_currentEvent.mouseButtons |= mouseButton;
            updateMousePos(x, y);
            updateHits();
            
            bool handled = false;
            for (unsigned int i = 0; i < m_receivers.size() && !handled; i++) {
                if (m_receivers[i]->mouseDown(m_currentEvent)) {
                    m_mouseUpReceiver = m_receivers[i];
                    handled = true;
                }
            }
            
            updateFeedback();
            return handled;
        }
        
        bool InputController::mouseUp(MouseButtonState mouseButton, float x, float y) {
            if (!m_documentViewHolder.valid())
                return false;
            
            updateMousePos(x, y);
            updateHits();
            
            bool handled = false;
            if (m_dragButtons != MouseButtons::MBNone) {
                if (m_dragReceiver != NULL)
                    m_dragReceiver->endDrag(m_currentEvent);
                if (m_mouseUpReceiver != NULL && m_mouseUpReceiver != m_dragReceiver)
                    m_mouseUpReceiver->mouseUp(m_currentEvent);
                m_dragReceiver = NULL;
                m_dragButtons = MouseButtons::MBNone;
                handled = true;
            } else {
                for (unsigned int i = 0; i < m_receivers.size() && !handled; i++)
                    if (m_receivers[i]->mouseUp(m_currentEvent))
                        handled = true;
            }
            
            m_mouseUpReceiver = NULL;
            m_currentEvent.mouseButtons &= ~mouseButton;
            updateFeedback();
            return handled;
        }
        
        void InputController::mouseMoved(float x, float y) {
            if (!m_documentViewHolder.valid())
                return;
            
            if (m_currentEvent.mouseButtons != MouseButtons::MBNone && m_dragButtons == MouseButtons::MBNone) {
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
            
            if (m_dragButtons != MouseButtons::MBNone && m_dragReceiver != NULL) {
                if (!m_dragReceiver->drag(m_currentEvent)) {
                    m_dragReceiver = NULL;
                    m_mouseUpReceiver = NULL;
                }
            }
            
            if (m_dragButtons == MouseButtons::MBNone || m_dragReceiver == NULL) {
                for (unsigned int i = 0; i < m_receivers.size(); i++)
                    m_receivers[i]->mouseMoved(m_currentEvent);
            }
            
            updateFeedback();
        }
        
        void InputController::scrolled(float dx, float dy) {
            if (!m_documentViewHolder.valid())
                return;
            
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
            updateFeedback();
        }

        void InputController::dragEnter(const String& payload, float x, float y) {
            if (!m_documentViewHolder.valid())
                return;

            updateMousePos(x, y);
            updateHits();

            for (unsigned int i = 0; i < m_dragTargetTools.size(); i++) {
                DragTargetTool* tool = m_dragTargetTools[i];
                if (tool->dragEnter(m_currentEvent, payload)) {
                    m_dragTargetReceiver = tool;
                    break;
                }
            }
        }
        
        void InputController::dragMove(const String& payload, float x, float y) {
            if (!m_documentViewHolder.valid())
                return;
            if (m_dragTargetReceiver == NULL)
                return;
            
            updateMousePos(x, y);
            updateHits();
            
            m_dragTargetReceiver->dragMove(m_currentEvent);
        }
        
        bool InputController::drop(const String& payload, float x, float y) {
            if (!m_documentViewHolder.valid())
                return false;
            if (m_dragTargetReceiver == NULL)
                return false;
            
            updateMousePos(x, y);
            updateHits();
            
            bool accept = m_dragTargetReceiver->drop(m_currentEvent);
            m_dragTargetReceiver = NULL;
            return accept;
        }
        
        void InputController::dragLeave() {
            if (!m_documentViewHolder.valid())
                return;
            if (m_dragTargetReceiver == NULL)
                return;
            
            m_dragTargetReceiver->dragLeave();
        }

        void InputController::changeEditState(const Model::EditStateChangeSet& changeSet) {
            ToolList::const_iterator toolIt, toolEnd;
            for (toolIt = m_receivers.begin(), toolEnd = m_receivers.end(); toolIt != toolEnd; ++toolIt) {
                Tool& tool = **toolIt;
                tool.changeEditState(changeSet);
            }
            
            DragTargetToolList::const_iterator dragToolIt, dragToolEnd;
            for (dragToolIt = m_dragTargetTools.begin(), dragToolEnd = m_dragTargetTools.end(); dragToolIt != dragToolEnd; ++dragToolIt) {
                DragTargetTool& tool = **dragToolIt;
                tool.changeEditState(changeSet);
            }

            updateFeedback();
        }

        void InputController::addFigure(Tool* tool, Renderer::Figure* figure) {
            m_figureHolder->addFigure(tool, figure);
        }
        
        void InputController::removeFigure(Tool* tool, Renderer::Figure* figure) {
            assert(m_figureHolder != NULL);
            m_figureHolder->removeFigure(tool, figure);
        }
        
        void InputController::deleteFigure(Tool* tool, Renderer::Figure* figure) {
            assert(m_figureHolder != NULL);
            m_figureHolder->deleteFigure(tool, figure);
        }

        bool InputController::toggleClipTool() {
            if (clipToolActive())
                return deactivateModalTool();
            return activateModalTool(m_clipTool);
        }
        
        void InputController::toggleClipSide() {
        }
        
        bool InputController::canPerformClip() {
            return m_clipTool->canPerformClip();
        }

        void InputController::performClip() {
        }

        bool InputController::clipToolActive() {
            return m_modalToolActive && m_receivers[ModalReceiverIndex] == m_clipTool;
        }

        bool InputController::deactivateModalTool() {
            if (m_modalToolActive) {
                if (m_receivers[ModalReceiverIndex]->deactivated(m_currentEvent)) {
                    m_receivers.erase(m_receivers.begin() + ModalReceiverIndex);
                    m_modalToolActive = false;
                    updateHits();
                    updateFeedback();
                    return true;
                }
                return false;
            }
            return true;
        }
    }
}