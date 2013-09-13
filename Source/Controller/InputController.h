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
#include "Model/Filter.h"
#include "Renderer/Figure.h"

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        class EntityDefinition;
    }

    namespace Renderer {
        class BoxGuideRenderer;
        class Camera;
        class RenderContext;
        class Vbo;
    }

    namespace View {
        class DocumentViewHolder;
    }

    namespace Controller {
        class CameraTool;
        class ClipTool;
        class Command;
        class CreateBrushTool;
        class CreateEntityTool;
        class FlyTool;
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
            CreateBrushTool* m_createBrushTool;
            CreateEntityTool* m_createEntityTool;
            FlyTool* m_flyTool;
            MoveObjectsTool* m_moveObjectsTool;
            MoveVerticesTool* m_moveVerticesTool;
            RotateObjectsTool* m_rotateObjectsTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            SetFaceAttributesTool* m_setFaceAttributesTool;
            SelectionTool* m_selectionTool;

            Tool* m_toolChain;
            Tool* m_dragTool;
            Tool* m_nextDropReceiver; // workaround for a bug in wxWidgets 2.9.4 on GTK2
            Tool* m_modalTool;
            bool m_cancelledDrag;

            wxPoint m_clickPos;
            bool m_discardNextMouseUp;

            void updateModalTool();
            void updateHits();
            void updateViews();

            Renderer::BoxGuideRenderer* m_selectionGuideRenderer;
            Model::SelectedFilter m_selectedFilter;

            void toggleTool(Tool* tool);
            
            // prevent copying
            InputController(const InputController& other);
            void operator= (const InputController& other);
        public:
            InputController(View::DocumentViewHolder& documentViewHolder);
            ~InputController();

            void modifierKeyDown(ModifierKeyState modifierKey);
            void modifierKeyUp(ModifierKeyState modifierKey);
            void resetModifierKeys();
            void clearModifierKeys();

            bool mouseDown(int x, int y, MouseButtonState mouseButton);
            bool mouseUp(int x, int y, MouseButtonState mouseButton);
            bool mouseDClick(int x, int y, MouseButtonState mouseButton);
            void mouseMove(int x, int y);
            void scroll(float x, float y);
            void cancelDrag();
            void endDrag();

            void dragEnter(const String& payload, int x, int y);
            void dragMove(const String& payload, int x, int y);
            bool drop(const String& payload, int x, int y);
            void dragLeave();
            
            bool navigateUp();
            void update(const Command& command);
            void cameraChanged();

            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);

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
            void toggleAxisRestriction();

            inline MoveVerticesTool& moveVerticesTool() const {
                return *m_moveVerticesTool;
            }
            
            void toggleMoveVerticesTool(size_t changeCount = 0);
            bool moveVerticesToolActive();

            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive();

            void deactivateAll();

            void createEntity(Model::EntityDefinition& definition);
            const Model::Entity* canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent);
            void reparentBrushes(const Model::BrushList& brushes, Model::Entity* newParent);

            inline InputState& inputState() {
                return m_inputState;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__InputController__) */
