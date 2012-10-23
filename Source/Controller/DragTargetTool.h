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

#ifndef TrenchBroom_DragTargetTool_h
#define TrenchBroom_DragTargetTool_h

#include "Controller/Input.h"
#include "Model/MapDocument.h"
#include "Renderer/MapRenderer.h"
#include "Utility/CommandProcessor.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

#include <wx/cmdproc.h>
#include <wx/event.h>

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
    }
    
    namespace Renderer {
        class Figure;
    }
    
    namespace Controller {
        class DragTargetTool {
        private:
            View::DocumentViewHolder& m_documentViewHolder;
            bool m_active;
            bool m_figureDataValid;
        protected:
            virtual bool handleDragEnter(InputEvent& event, const String& payload) { return false; }
            virtual void handleDragMove(InputEvent& event) { }
            virtual void handleDragLeave() { }
            virtual bool handleDrop(InputEvent& event) { return false; }
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
            
            inline void postCommand(wxCommand* command) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                document.GetCommandProcessor()->Submit(command);
            }
            
            inline void addFigure(Renderer::Figure* figure) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Renderer::MapRenderer& renderer = m_documentViewHolder.view().renderer();
                renderer.addFigure(figure);
            }
            
            inline void removeFigure(Renderer::Figure* figure) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Renderer::MapRenderer& renderer = m_documentViewHolder.view().renderer();
                renderer.removeFigure(figure);
            }
            
            inline void deleteFigure(Renderer::Figure* figure) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Renderer::MapRenderer& renderer = m_documentViewHolder.view().renderer();
                renderer.deleteFigure(figure);
            }
            
            inline View::DocumentViewHolder& documentViewHolder() {
                return m_documentViewHolder;
            }
        public:
            DragTargetTool(View::DocumentViewHolder& documentViewHolder) :
            m_documentViewHolder(documentViewHolder),
            m_active(false),
            m_figureDataValid(false) {}
            
            virtual ~DragTargetTool() {}

            inline bool dragEnter(InputEvent& event, const String& payload) {
                m_active = handleDragEnter(event, payload);
                return m_active;
            }
            
            inline void dragMove(InputEvent& event) {
                assert(m_active);
                handleDragMove(event);
            }
            
            inline bool drop(InputEvent& event) {
                assert(m_active);
                return handleDrop(event);
            }
            
            inline void dragLeave() {
                assert(m_active);
                handleDragLeave();
                m_active = false;
            }

            void changeEditState(const Model::EditStateChangeSet& changeSet) {
                handleChangeEditState(changeSet);
            }
        };
        
        typedef std::vector<DragTargetTool*> DragTargetToolList;
    }
}

#endif
