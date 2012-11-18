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

#include "Controller/Input.h"
#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Renderer/Figure.h"

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class CameraTool;
        class CreateBrushTool;
        class CreateEntityTool;
        class MoveObjectsTool;
        class RotateObjectsTool;
        class SelectionTool;
        class Tool;
    
        class InputController {
        private:
            View::DocumentViewHolder& m_documentViewHolder;
            InputState m_inputState;
            
            CameraTool* m_cameraTool;
            CreateBrushTool* m_createBrushTool;
            CreateEntityTool* m_createEntityTool;
            MoveObjectsTool* m_moveObjectsTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
            
            Tool* m_toolChain;
            Tool* m_dragTool;
            Tool* m_modalTool;
            
            void updateModalTool();
            void updateHits();
            void updateState();
        public:
            InputController(View::DocumentViewHolder& documentViewHolder);
            ~InputController();
            
            void modifierKeyDown(ModifierKeyState modifierKey);
            void modifierKeyUp(ModifierKeyState modifierKey);
            
            bool mouseDown(MouseButtonState mouseButton);
            bool mouseUp(MouseButtonState mouseButton);
            void mouseMove(int x, int y);
            void scroll(float x, float y);
            void cancelDrag();
            
            void dragEnter(const String& payload, int x, int y);
            void dragMove(const String& payload, int x, int y);
            bool drop(const String& payload, int x, int y);
            void dragLeave();
            
            void objectsChange();
            void editStateChange(const Model::EditStateChangeSet& changeSet);
            void cameraChange();

            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);
        };

        class InputControllerFigure : public Renderer::Figure {
        private:
            InputController& m_inputController;
        public:
            InputControllerFigure(InputController& inputController);
            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);
        };
        
    }
}

#endif /* defined(__TrenchBroom__InputController__) */
