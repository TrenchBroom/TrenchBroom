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
#include <map>
#include <string>
#include "Controller/DragTargetTool.h"
#include "Controller/Tool.h"
#include "Utilities/SharedPointer.h"

namespace TrenchBroom {
    namespace Controller {
        
        typedef enum {
            TB_MS_NONE,
            TB_MS_LEFT,
            TB_MS_RIGHT
        } EMouseStatus;
        
        class Tool;
        class ToolEvent;
        class DragTargetTool;
        class DragInfo;
        class Editor;

        class InputController {
        private:
            Editor& m_editor;
            ToolEvent m_currentEvent;
            EMouseStatus m_dragStatus;
            
            // TODO shared pointer:
            typedef std::tr1::shared_ptr<Tool> ToolPtr;
            typedef std::vector<ToolPtr> ToolList;
            ToolList m_receiverChain;
            ToolPtr m_dragScrollReceiver;
            int m_modalReceiverIndex;

            ToolPtr m_moveVertexTool;
            ToolPtr m_moveEdgeTool;
            ToolPtr m_moveFaceTool;
            
            // TODO shared pointer:
            DragInfo m_currentDragInfo;
            typedef std::tr1::shared_ptr<DragTargetTool> DragTargetToolPtr;
            typedef std::map<std::string, DragTargetToolPtr> DragTargetToolMap;
            DragTargetToolMap m_dragTargetTools;
            
            void updateHits();
            void toggleModalTool(const ToolPtr& tool, unsigned int index);
            bool modalToolActive(const ToolPtr& tool);
        public:
            InputController(Editor& editor);
            ~InputController();
            
            void toggleMoveVertexTool();
            void toggleMoveEdgeTool();
            void toggleMoveFaceTool();
            void toggleClipTool() {}
            
            bool moveVertexToolActive();
            bool moveEdgeToolActive();
            bool moveFaceToolActive();
            bool clipToolActive() {return false;}
            
            bool key(wchar_t c);
            void modifierKeyDown(EModifierKeys modifierKey);
            void modifierKeyUp(EModifierKeys modifierKey);
            void mouseDown(EMouseButton mouseButton);
            void mouseUp(EMouseButton mouseButton);
            void mouseMoved(float x, float y, float dx, float dy);
            void scrolled(float dx, float dy);
            
            bool dragEnter(const std::string& name, void* payload, float x, float y);
            void dragLeave(const std::string& name, void* payload);
            bool dragMove(const std::string& name, void* payload, float x, float y);
            bool acceptDrag(const std::string& name, void* payload);
            bool handleDrop(const std::string& name, void* payload, float x, float y);
        };
    }
}

#endif
