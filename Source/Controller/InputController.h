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
        class EntityDefinition;
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
        class ClipTool;
        class CreateBrushTool;
        class CreateEntityTool;
        class MoveObjectsTool;
        class MoveVerticesTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class SelectionTool;
        class SetFaceAttributesTool;
        class Tool;
    
        class InputController {
        private:
            View::DocumentViewHolder& m_documentViewHolder;
            InputState m_inputState;
            
            CameraTool* m_cameraTool;
            ClipTool* m_clipTool;
            MoveVerticesTool* m_moveVerticesTool;
            CreateBrushTool* m_createBrushTool;
            CreateEntityTool* m_createEntityTool;
            MoveObjectsTool* m_moveObjectsTool;
            RotateObjectsTool* m_rotateObjectsTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            SetFaceAttributesTool* m_setFaceAttributesTool;
            SelectionTool* m_selectionTool;
            
            Tool* m_toolChain;
            Tool* m_dragTool;
            Tool* m_modalTool;
            bool m_cancelledDrag;
            
            wxPoint m_clickPos;
            bool m_discardNextMouseUp;
            
            void updateModalTool();
            void updateHits();
            void updateViews();
        public:
            InputController(View::DocumentViewHolder& documentViewHolder);
            ~InputController();
            
            void modifierKeyDown(ModifierKeyState modifierKey);
            void modifierKeyUp(ModifierKeyState modifierKey);
            
            bool mouseDown(MouseButtonState mouseButton);
            bool mouseUp(MouseButtonState mouseButton);
            bool mouseDClick(MouseButtonState mouseButton);
            void mouseMove(int x, int y);
            void scroll(float x, float y);
            void cancelDrag();
            void endDrag();
            
            void dragEnter(const String& payload, int x, int y);
            void dragMove(const String& payload, int x, int y);
            bool drop(const String& payload, int x, int y);
            void dragLeave();
            
            void objectsChange();
            void editStateChange(const Model::EditStateChangeSet& changeSet);
            void cameraChange();
            void gridChange();

            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);
            void freeRenderResources();
            
            inline ClipTool& clipTool() const {
                return *m_clipTool;
            }
            
            void toggleClipTool();
            bool clipToolActive();
            void toggleClipSide();
            bool canDeleteClipPoint();
            void deleteClipPoint();
            bool canPerformClip();
            void performClip();
            
            inline MoveVerticesTool& moveVerticesTool() const {
                return *m_moveVerticesTool;
            }
            
            void toggleMoveVerticesTool();
            bool moveVerticesToolActive();
            
            void deactivateAll();
            
            void createEntity(Model::EntityDefinition& definition);
            Model::Entity* canReparentBrushes(const Model::BrushList& brushes);
            void reparentBrushes(const Model::BrushList& brushes);
            
        };

        class InputControllerFigure : public Renderer::Figure {
        private:
            InputController& m_inputController;
        public:
            InputControllerFigure(InputController& inputController);
            ~InputControllerFigure();
            
            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);
        };
        
    }
}

#endif /* defined(__TrenchBroom__InputController__) */
