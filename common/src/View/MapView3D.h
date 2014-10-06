/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapView3D__
#define __TrenchBroom__MapView3D__

#include "MathUtils.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/Action.h"
#include "View/GLContextHolder.h"
#include "View/MapView.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ViewTypes.h"

class wxBookCtrlBase;

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class Compass;
        class MapRenderer;
        class RenderBatch;
        class RenderContext;
        class Vbo;
    }
    
    namespace View {
        class CameraTool;
        class MoveObjectsTool;
        class SelectionTool;
        class Selection;
        class MovementRestriction;
        
        class MapView3D : public MapView, public RenderView, public ToolBoxHelper {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            MovementRestriction* m_movementRestriction;
            
            Renderer::Vbo* m_vbo;
            Renderer::MapRenderer& m_renderer;
            Renderer::PerspectiveCamera m_camera;
            Renderer::Compass* m_compass;
            
            ToolBox m_toolBox;
            CameraTool* m_cameraTool;
            MoveObjectsTool* m_moveObjectsTool;
            SelectionTool* m_selectionTool;
        public:
            MapView3D(wxWindow* parent, Logger* logger, wxBookCtrlBase* toolBook, MapDocumentWPtr document, Renderer::MapRenderer& renderer);
            ~MapView3D();
        private:
            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Selection& selection);
        private: // interaction events
            void bindEvents();
            
            void OnMoveObjectsForward(wxCommandEvent& event);
            void OnMoveObjectsBackward(wxCommandEvent& event);
            void OnMoveObjectsLeft(wxCommandEvent& event);
            void OnMoveObjectsRight(wxCommandEvent& event);
            void OnMoveObjectsUp(wxCommandEvent& event);
            void OnMoveObjectsDown(wxCommandEvent& event);

            void OnRollObjectsCW(wxCommandEvent& event);
            void OnRollObjectsCCW(wxCommandEvent& event);
            void OnPitchObjectsCW(wxCommandEvent& event);
            void OnPitchObjectsCCW(wxCommandEvent& event);
            void OnYawObjectsCW(wxCommandEvent& event);
            void OnYawObjectsCCW(wxCommandEvent& event);
            
            void OnFlipObjectsH(wxCommandEvent& event);
            void OnFlipObjectsV(wxCommandEvent& event);
        private: // interaction event helper methods
            void moveObjects(Math::Direction direction);
            Vec3 moveDirection(Math::Direction direction) const;
            void rotateObjects(Math::RotationAxis axis, bool clockwise);
            Vec3 rotationAxis(Math::RotationAxis axis, bool clockwise) const;
            void flipObjects(Math::Direction direction);
        private: // other events
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        private: // accelerator table management
            void updateAcceleratorTable(bool hasFocus);
            Action::Context actionContext() const;
        private: // implement RenderView
            void doInitializeGL();
            void doUpdateViewport(int x, int y, int width, int height);
            bool doShouldRenderFocusIndicator() const;
            void doRender();
            void setupGL(Renderer::RenderContext& renderContext);
            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderCompass(Renderer::RenderBatch& renderBatch);
        private: // implement ToolBoxHelper
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
            void doShowPopupMenu();
        private: // Tool related methods
            void createTools(wxBookCtrlBase* toolBook);
            void destroyTools();
        private:
            static const GLContextHolder::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView3D__) */
