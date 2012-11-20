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
#include "Controller/CreateBrushTool.h"
#include "Controller/CreateEntityTool.h"
#include "Controller/MoveObjectsTool.h"
#include "Controller/RotateObjectsTool.h"
#include "Controller/SelectionTool.h"
#include "Model/MapDocument.h"
#include "Renderer/MapRenderer.h"

namespace TrenchBroom {
    namespace Controller {
        void InputController::updateModalTool() {
            if (m_dragTool != NULL)
                return;
            
            if (m_modalTool == NULL) {
                m_modalTool = m_toolChain->modalTool(m_inputState);
                if (m_modalTool != NULL) {
                    m_toolChain->setSuppressed(m_inputState, true, m_modalTool);
                    updateHits();
                }
            } else {
                if (!m_modalTool->isModal(m_inputState) || m_modalTool != m_toolChain->modalTool(m_inputState)) {
                    m_toolChain->setSuppressed(m_inputState, false);
                    updateHits();
                    m_modalTool = m_toolChain->modalTool(m_inputState);
                    if (m_modalTool != NULL) {
                        m_toolChain->setSuppressed(m_inputState, true, m_modalTool);
                        updateHits();
                    }
                }
            }
        }

        void InputController::updateHits() {
            m_inputState.invalidate();
            if (m_dragTool == NULL)
                m_toolChain->updateHits(m_inputState);
        }

        void InputController::updateState() {
            if (m_documentViewHolder.valid() && m_toolChain->updateState(m_inputState))
                m_documentViewHolder.document().UpdateAllViews();
        }
        
        InputController::InputController(View::DocumentViewHolder& documentViewHolder) :
        m_documentViewHolder(documentViewHolder),
        m_inputState(m_documentViewHolder.view().camera(), m_documentViewHolder.document().picker()),
        m_cameraTool(NULL),
        m_clipTool(NULL),
        m_createBrushTool(NULL),
        m_createEntityTool(NULL),
        m_moveObjectsTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_toolChain(NULL),
        m_dragTool(NULL),
        m_modalTool(NULL) {
            m_cameraTool = new CameraTool(m_documentViewHolder);
            m_clipTool = new ClipTool(m_documentViewHolder);
            m_createBrushTool = new CreateBrushTool(m_documentViewHolder);
            m_createEntityTool = new CreateEntityTool(m_documentViewHolder);
            m_moveObjectsTool = new MoveObjectsTool(m_documentViewHolder, 64.0f, 32.0f);
            m_rotateObjectsTool = new RotateObjectsTool(m_documentViewHolder, 64.0f, 32.0f, 5.0f);
            m_selectionTool = new SelectionTool(m_documentViewHolder);

            m_cameraTool->setNextTool(m_clipTool);
            m_clipTool->setNextTool(m_createEntityTool);
            m_createEntityTool->setNextTool(m_createBrushTool);
            m_createBrushTool->setNextTool(m_moveObjectsTool);
            m_moveObjectsTool->setNextTool(m_rotateObjectsTool);
            m_rotateObjectsTool->setNextTool(m_selectionTool);
            m_toolChain = m_cameraTool;
            
            m_createBrushTool->activate(m_inputState);
            m_moveObjectsTool->activate(m_inputState);
            m_rotateObjectsTool->activate(m_inputState);
            
            m_documentViewHolder.view().renderer().addFigure(new InputControllerFigure(*this));
        }
        
        InputController::~InputController() {
            m_toolChain = NULL;
            m_dragTool = NULL;
            m_modalTool = NULL;
            
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_clipTool;
            m_clipTool = NULL;
            delete m_createBrushTool;
            m_createBrushTool = NULL;
            delete m_createEntityTool;
            m_createEntityTool = NULL;
            delete m_moveObjectsTool;
            m_moveObjectsTool = NULL;
            delete m_rotateObjectsTool;
            m_rotateObjectsTool = NULL;
            delete m_selectionTool;
            m_selectionTool = NULL;
        }

        void InputController::modifierKeyDown(ModifierKeyState modifierKey) {
            updateHits();
            m_toolChain->modifierKeyChange(m_inputState);
            updateModalTool();
            updateState();
        }
        
        void InputController::modifierKeyUp(ModifierKeyState modifierKey) {
            updateHits();
            m_toolChain->modifierKeyChange(m_inputState);
            updateModalTool();
            updateState();
        }
        
        bool InputController::mouseDown(MouseButtonState mouseButton) {
            if (m_dragTool != NULL)
                return false;
            
            m_inputState.mouseDown(mouseButton);
            updateHits();
            bool handled = m_toolChain->mouseDown(m_inputState) != NULL;
            updateModalTool();
            updateState();
            return handled;
        }
        
        bool InputController::mouseUp(MouseButtonState mouseButton) {
            bool handled = false;
            if (m_dragTool != NULL) {
                m_dragTool->endDrag(m_inputState);
                m_dragTool = NULL;
                m_inputState.mouseUp(mouseButton);
                handled = true;
            } else {
                handled = m_toolChain->mouseUp(m_inputState) != NULL;
                m_inputState.mouseUp(mouseButton);
            }
            
            updateHits();
            updateModalTool();
            updateState();
            return handled;
        }
        
        void InputController::mouseMove(int x, int y) {
            m_inputState.mouseMove(x, y);
            updateHits();
            
            if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                if (m_dragTool == NULL)
                    m_dragTool = m_toolChain->startDrag(m_inputState);
                if (m_dragTool != NULL)
                    m_dragTool->drag(m_inputState);
                else
                    m_toolChain->mouseMove(m_inputState);
            } else {
                m_toolChain->mouseMove(m_inputState);
            }
            
            updateModalTool();
            updateState();
        }

        void InputController::scroll(float x, float y) {
            m_inputState.scroll(x, y);
            updateHits();
            
            if (m_dragTool != NULL)
                m_dragTool->scroll(m_inputState);
            else
                m_toolChain->scroll(m_inputState);
            
            updateModalTool();
            updateState();
        }
        
        void InputController::cancelDrag() {
            if (m_dragTool != NULL) {
                m_dragTool->cancelDrag(m_inputState);
                m_dragTool = NULL;
                m_inputState.mouseUp(m_inputState.mouseButtons());
            }

            updateHits();
            updateModalTool();
            updateState();
        }
        
        void InputController::dragEnter(const String& payload, int x, int y) {
            assert(m_dragTool == NULL);
            
            m_inputState.mouseMove(x, y);
            m_createEntityTool->activate(m_inputState);

            updateHits();
            Tool* dragTool = m_toolChain->dragEnter(m_inputState, payload);
            updateModalTool();
            m_dragTool = dragTool;
            updateState();
        }
        
        void InputController::dragMove(const String& payload, int x, int y) {
            if (m_dragTool == NULL)
                return;

            m_inputState.mouseMove(x, y);
            updateHits();
            m_dragTool->dragMove(m_inputState);
            updateState();
        }
        
        bool InputController::drop(const String& payload, int x, int y) {
            if (m_dragTool == NULL)
                return false;
            
            updateHits();
            bool success = m_dragTool->dragDrop(m_inputState);
            m_createEntityTool->deactivate(m_inputState);
            m_dragTool = NULL;

            updateHits();
            updateModalTool();
            updateState();

            return success;
        }
        
        void InputController::dragLeave() {
            if (m_dragTool == NULL)
                return;
            
            updateHits();
            m_dragTool->dragLeave(m_inputState);
            m_createEntityTool->deactivate(m_inputState);
            m_dragTool = NULL;

            updateHits();
            updateModalTool();
            updateState();
        }

        void InputController::objectsChange() {
            updateHits();
            m_toolChain->objectsChange(m_inputState);
            updateModalTool();
            updateState();
        }

        void InputController::editStateChange(const Model::EditStateChangeSet& changeSet) {
            updateHits();
            m_toolChain->editStateChange(m_inputState, changeSet);
            updateModalTool();
            updateState();
        }
        
        void InputController::cameraChange() {
            updateHits();
            m_toolChain->cameraChange(m_inputState);
            updateModalTool();
            updateState();
        }

        void InputController::render(Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            m_toolChain->render(m_inputState, vbo, context);
        }

        void InputController::toggleClipTool() {
            if (m_clipTool->active())
                m_clipTool->deactivate(m_inputState);
            else
                m_clipTool->activate(m_inputState);
            updateHits();
            updateModalTool();
            updateState();
            m_documentViewHolder.document().UpdateAllViews();
        }

        bool InputController::clipToolActive() {
            return m_clipTool->active();
        }

        InputControllerFigure::InputControllerFigure(InputController& inputController) :
        m_inputController(inputController) {}
        
        void InputControllerFigure::render(Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            m_inputController.render(vbo, context);
        }
    }
}