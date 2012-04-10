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

#ifndef TrenchBroom_InputController_h
#define TrenchBroom_InputController_h

#include <vector>
#include "Tool.h"

using namespace std;

namespace TrenchBroom {
    namespace Controller {
        
        typedef enum {
            MS_NONE,
            MS_LEFT,
            MS_RIGHT
        } EMouseStatus;
        
        class Tool;
        class ToolEvent;
        class CameraTool;
        class SelectionTool;
        class Editor;

        class InputController {
        private:
            Editor& m_editor;
            ToolEvent m_currentEvent;
            EMouseStatus m_dragStatus;
            
            vector<Tool*> m_receiverChain;
            Tool* m_dragScrollReceiver;
            int m_modalReceiverIndex;
            
            CameraTool* m_cameraTool;
            SelectionTool* m_selectionTool;
            
            void updateHits();
        public:
            InputController(Editor& editor);
            ~InputController();
            void modifierKeyDown(EModifierKeys modifierKey);
            void modifierKeyUp(EModifierKeys modifierKey);
            void mouseDown(EMouseButton mouseButton);
            void mouseUp(EMouseButton mouseButton);
            void mouseMoved(float x, float y, float dx, float dy);
            void scrolled(float dx, float dy);
        };
    }
}

#endif
