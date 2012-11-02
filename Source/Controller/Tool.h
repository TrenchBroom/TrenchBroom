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

#ifndef TrenchBroom_Tool_h
#define TrenchBroom_Tool_h

#include "Controller/Input.h"
#include "Controller/InputController.h"
#include "Model/MapDocument.h"
#include "Renderer/MapRenderer.h"
#include "Utility/CommandProcessor.h"
#include "Utility/VecMath.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

#include <wx/cmdproc.h>
#include <wx/event.h>

#include <cstdio>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
    }
    
    namespace Renderer {
        class Figure;
    }
    
    namespace Controller {
        class Tool {
        public:
            enum State {
                Default,
                MouseDown,
                Drag,
                Scroll
            };
        private:
            View::DocumentViewHolder& m_documentViewHolder;
            InputController& m_inputController;
            State m_state;
            bool m_active;
        protected:
            virtual bool handleActivated(InputEvent& event) { return false; }
            virtual bool handleDeactivated(InputEvent& event) { return false; }
            virtual void handleModifierKeyChanged(InputEvent& event) {}
            virtual bool handleMouseDown(InputEvent& event) { return false; }
            virtual bool handleMouseUp(InputEvent& event) { return false; }
            virtual bool handleMouseMoved(InputEvent& event) { return false; }
            virtual bool handleScrolled(InputEvent& event) { return false; }

            virtual bool handleBeginDrag(InputEvent& event) { return false; }
            virtual bool handleDrag(InputEvent& event) { return false; }
            virtual void handleEndDrag(InputEvent& event) {}
            
            virtual void handleChangeEditState(const Model::EditStateChangeSet& changeSet) {}

            inline void postEvent(wxEvent& event) {
                if (!m_documentViewHolder.valid())
                    return;
                
                View::EditorView& view = m_documentViewHolder.view();
                event.SetEventObject(&view);
                view.ProcessEvent(event);
            }
            
            inline void BeginCommandGroup(const wxString& name) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::BeginGroup(document.GetCommandProcessor(), name);
            }
            
            inline void EndCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::EndGroup(document.GetCommandProcessor());
            }

            inline void RollbackCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::RollbackGroup(document.GetCommandProcessor());
            }
            
            inline void DiscardCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::DiscardGroup(document.GetCommandProcessor());
            }
            
            inline void postCommand(wxCommand* command) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                document.GetCommandProcessor()->Submit(command);
            }

            inline View::DocumentViewHolder& documentViewHolder() {
                return m_documentViewHolder;
            }
            
            inline void addFigure(Renderer::Figure* figure) {
                m_inputController.addFigure(this, figure);
            }
            
            inline void removeFigure(Renderer::Figure* figure) {
                m_inputController.removeFigure(this, figure);
            }
            
            inline void deleteFigure(Renderer::Figure* figure) {
                m_inputController.deleteFigure(this, figure);
            }
            
        public:
            Tool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
            m_documentViewHolder(documentViewHolder),
            m_inputController(inputController),
            m_state(Default),
            m_active(false) {}
            
            virtual ~Tool() {}
            
            virtual void updateHits(InputEvent& event) {}
            
            virtual bool suppressOtherFeedback(InputEvent& event) { return false; }
            
            virtual bool updateFeedback(InputEvent& event) { return false; }
            
            inline State state() const {
                return m_state;
            }
            
            inline bool active() const {
                return m_active;
            }
            
            bool activated(InputEvent& event) {
                if (handleActivated(event)) {
                    m_active = true;
                    return true;
                }
                
                return false;
            }
            
            bool deactivated(InputEvent& event) {
                if (handleDeactivated(event)) {
                    m_active = false;
                    m_state = Default;
                    return true;
                }
                
                return false;
            }
            
            void modifierKeyChanged(InputEvent& event) {
                handleModifierKeyChanged(event);
            }
            
            bool mouseDown(InputEvent& event) {
                if (handleMouseDown(event)) {
                    m_state = MouseDown;
                    return true;
                }
                
                return false;
            }
            
            bool mouseUp(InputEvent& event) {
                if (handleMouseUp(event)) {
                    m_state = Default;
                    return true;
                }
                
                return false;
            }
            
            bool mouseMoved(InputEvent& event) {
                return handleMouseMoved(event);
            }
            
            bool scrolled(InputEvent& event) {
                return handleScrolled(event);
            }
            
            bool beginDrag(InputEvent& event) {
                if (handleBeginDrag(event)) {
                    m_state = Drag;
                    return true;
                }
                
                return false;
            }
            
            bool drag(InputEvent& event) {
                if (handleDrag(event)) {
                    return true;
                } else {
                    endDrag(event);
                    return false;
                }
            }
            
            void endDrag(InputEvent& event) {
                handleEndDrag(event);
                m_state = Default;
            }
            
            static bool noModifierPressed(InputEvent& event) {
                return event.modifierKeys() == ModifierKeys::MKNone;
            }

            void changeEditState(const Model::EditStateChangeSet& changeSet) {
                handleChangeEditState(changeSet);
            }
        };
    }
}


#endif
