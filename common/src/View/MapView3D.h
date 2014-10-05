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

#include "Renderer/PerspectiveCamera.h"
#include "View/Action.h"
#include "View/GLContextHolder.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ViewTypes.h"

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
        class SelectionTool;
        class Selection;
        class MovementRestriction;
        
        class MapView3D : public RenderView, public ToolBoxHelper {
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
            SelectionTool* m_selectionTool;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, Renderer::MapRenderer& renderer);
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
        private: // interaction event helper methods
            void moveObjects(Math::Direction direction);
            Vec3 moveDirection(Math::Direction direction) const;
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
            void renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderCompass(Renderer::RenderBatch& renderBatch);
        private: // implement ToolBoxHelper
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
            void doShowPopupMenu();
        private: // Tool related methods
            void createTools();
            void destroyTools();
        private:
            static const GLContextHolder::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView3D__) */
