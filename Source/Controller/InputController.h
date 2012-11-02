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

#ifndef __TrenchBroom__InputController__
#define __TrenchBroom__InputController__

#include "Controller/DragTargetTool.h"
#include "Controller/Input.h"

#include <vector>

class wxEvtHandler;

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        class MapDocument;
        class Picker;
    }
    
    namespace Renderer {
        class Camera;
        class InputControllerFeedbackFigure;
        class Figure;
    }
    
    namespace View {
        class DocumentViewHolder;
        class EditorView;
    }
    
    namespace Controller {
        class Tool;
        
        class InputController {
        protected:
            typedef std::vector<Tool*> ToolList;

            View::DocumentViewHolder& m_documentViewHolder;
            
            InputEvent m_currentEvent;
            MouseButtonState m_dragButtons;
            ToolList m_receivers;
            Tool* m_dragReceiver;
            Tool* m_mouseUpReceiver;
            Tool* m_singleFeedbackProvider;
            int m_modalReceiverIndex;

            DragTargetToolList m_dragTargetTools;
            DragTargetTool* m_dragTargetReceiver;
            
            Renderer::InputControllerFeedbackFigure* m_figureHolder;

            void updateHits();
            void updateFeedback();
            void updateMousePos(float x, float y);
        public:
            InputController(View::DocumentViewHolder& documentViewHolder);
            ~InputController();
            
            void modifierKeyDown(ModifierKeyState modifierKey);
            void modifierKeyUp(ModifierKeyState modifierKey);
            bool mouseDown(MouseButtonState mouseButton, float x, float y);
            bool mouseUp(MouseButtonState mouseButton, float x, float y);
            void mouseMoved(float x, float y);
            void scrolled(float dx, float dy);
            
            void dragEnter(const String& payload, float x, float y);
            void dragMove(const String& payload, float x, float y);
            bool drop(const String& payload, float x, float y);
            void dragLeave();
            
            void changeEditState(const Model::EditStateChangeSet& changeSet);
            
            void addFigure(Tool* tool, Renderer::Figure* figure);
            void removeFigure(Tool* tool, Renderer::Figure* figure);
            void deleteFigure(Tool* tool, Renderer::Figure* figure);
        };
    }
}

#endif /* defined(__TrenchBroom__InputController__) */
