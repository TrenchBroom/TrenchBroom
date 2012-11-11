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
#include "Controller/MoveObjectsTool.h"
#include "Controller/SelectionTool.h"
#include "Model/MapDocument.h"
#include "Renderer/MapRenderer.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateViews() {
            if (m_documentViewHolder.valid() && m_toolChain->needsUpdate(m_inputState))
                m_documentViewHolder.document().UpdateAllViews();
        }

        void InputController::updateModalTool() {
            if (m_dragTool != NULL)
                return;
            
            if (m_modalTool == NULL) {
                m_modalTool = m_toolChain->modalTool(m_inputState);
                if (m_modalTool != NULL) {
                    // deactivate all tools but the modal tool
                }
            } else {
                if (!m_modalTool->isModal(m_inputState)) {
                    m_modalTool = m_toolChain->modalTool(m_inputState);
                    if (m_modalTool != NULL) {
                        // deactivate all tools but the new modal tool
                    }
                }
            }
        }

        void InputController::updateHits() {
            m_inputState.invalidate();
            if (m_dragTool == NULL)
                m_toolChain->updateHits(m_inputState);
        }

        InputController::InputController(View::DocumentViewHolder& documentViewHolder) :
        m_documentViewHolder(documentViewHolder),
        m_inputState(m_documentViewHolder.view().camera(), m_documentViewHolder.document().picker()),
        m_cameraTool(NULL),
        m_moveObjectsTool(NULL),
        m_selectionTool(NULL),
        m_toolChain(NULL),
        m_dragTool(NULL),
        m_modalTool(NULL) {
            m_cameraTool = new CameraTool(m_documentViewHolder);
            m_selectionTool = new SelectionTool(m_documentViewHolder);
            m_moveObjectsTool = new MoveObjectsTool(m_documentViewHolder, 64.0f, 32.0f);
            m_cameraTool->setNextTool(m_moveObjectsTool);
            m_moveObjectsTool->setNextTool(m_selectionTool);
            m_toolChain = m_cameraTool;
            m_moveObjectsTool->activate(m_inputState);
            m_documentViewHolder.view().renderer().addFigure(new InputControllerFigure(*this));
        }
        
        InputController::~InputController() {
            m_toolChain = NULL;
            m_dragTool = NULL;
            m_modalTool = NULL;

            if (m_cameraTool != NULL) {
                delete m_cameraTool;
                m_cameraTool = NULL;
            }
            if (m_moveObjectsTool != NULL) {
                delete m_moveObjectsTool;
                m_moveObjectsTool = NULL;
            }
            if (m_selectionTool != NULL) {
                delete m_selectionTool;
                m_selectionTool = NULL;
            }
        }

        void InputController::modifierKeyDown(ModifierKeyState modifierKey) {
            updateHits();
            m_toolChain->modifierKeyChange(m_inputState);
            updateModalTool();
            updateViews();
        }
        
        void InputController::modifierKeyUp(ModifierKeyState modifierKey) {
            m_inputState.invalidate();
            m_toolChain->modifierKeyChange(m_inputState);
            updateModalTool();
            updateViews();
        }
        
        bool InputController::mouseDown(MouseButtonState mouseButton) {
            if (m_dragTool != NULL)
                return false;
            
            updateHits();
            m_inputState.mouseDown(mouseButton);
            bool handled = m_toolChain->mouseDown(m_inputState) != NULL;
            updateModalTool();
            updateViews();
            return handled;
        }
        
        bool InputController::mouseUp(MouseButtonState mouseButton) {
            bool handled = false;
            if (m_dragTool != NULL) {
                m_dragTool->endDrag(m_inputState);
                m_dragTool = NULL;
                m_inputState.mouseUp(mouseButton);
                updateHits();
                handled = true;
            } else {
                updateHits();
                handled = m_toolChain->mouseUp(m_inputState) != NULL;
                m_inputState.mouseUp(mouseButton);
            }
            
            updateModalTool();
            updateViews();
            return handled;
        }
        
        void InputController::mouseMove(int x, int y) {
            updateHits();
            
            if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                if (m_dragTool == NULL)
                    m_dragTool = m_toolChain->startDrag(m_inputState);
                m_inputState.mouseMove(x, y);
                if (m_dragTool != NULL)
                    m_dragTool->drag(m_inputState);
                else
                    m_toolChain->mouseMove(m_inputState);
            } else {
                m_inputState.mouseMove(x, y);
                m_toolChain->mouseMove(m_inputState);
            }
            
            updateModalTool();
            updateViews();
        }

        void InputController::scroll(float x, float y) {
            updateHits();
            m_inputState.scroll(x, y);
            if (m_dragTool != NULL)
                m_dragTool->scroll(m_inputState);
            else
                m_toolChain->scroll(m_inputState);
            updateModalTool();
            updateViews();
        }
        
        void InputController::cancelDrag() {
            if (m_dragTool != NULL) {
                m_dragTool->cancelDrag(m_inputState);
                m_dragTool = NULL;
                m_inputState.mouseUp(m_inputState.mouseButtons());
            }
            updateHits();
            updateModalTool();
            updateViews();
        }
        
        void InputController::dragEnter(const String& payload, int x, int y) {
        }
        
        void InputController::dragMove(const String& payload, int x, int y) {
        }
        
        bool InputController::drop(const String& payload, int x, int y) {
            return false;
        }
        
        void InputController::dragLeave() {
        }

        void InputController::editStateChange(const Model::EditStateChangeSet& changeSet) {
            updateHits();
            m_toolChain->editStateChange(m_inputState, changeSet);
            updateModalTool();
            updateViews();
        }
        
        void InputController::cameraChange() {
            updateHits();
            m_toolChain->cameraChange(m_inputState);
            updateModalTool();
            updateViews();
        }

        void InputController::render(Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            m_toolChain->render(m_inputState, vbo, context);
        }

        InputControllerFigure::InputControllerFigure(InputController& inputController) :
        m_inputController(inputController) {}
        
        void InputControllerFigure::render(Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            m_inputController.render(vbo, context);
        }
    }
}