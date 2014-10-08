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
#include "View/RenderView.h"
#include "View/ToolBoxConnector.h"
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
        class AnimationManager;
        class Command;
        class MapViewToolBox;
        class MovementRestriction;
        class Selection;
        class Tool;
        
        class MapView3D : public RenderView, public ToolBoxConnector {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            MapViewToolBox& m_toolBox;
            
            AnimationManager* m_animationManager;

            Renderer::Vbo* m_vbo;
            Renderer::MapRenderer& m_renderer;
            Renderer::PerspectiveCamera m_camera;
            Renderer::Compass* m_compass;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer);
            ~MapView3D();
            
            Renderer::Camera* camera();
        private:
            void bindObservers();
            void unbindObservers();
            
            void toolChanged(Tool* tool);
            void commandProcessed(Command* command);
            void selectionDidChange(const Selection& selection);
        private: // interaction events
            void bindEvents();
            
            void OnDeleteObjects(wxCommandEvent& event);
            
            void OnMoveObjectsForward(wxCommandEvent& event);
            void OnMoveObjectsBackward(wxCommandEvent& event);
            void OnMoveObjectsLeft(wxCommandEvent& event);
            void OnMoveObjectsRight(wxCommandEvent& event);
            void OnMoveObjectsUp(wxCommandEvent& event);
            void OnMoveObjectsDown(wxCommandEvent& event);

            void OnDuplicateObjects(wxCommandEvent& event);

            void OnDuplicateObjectsForward(wxCommandEvent& event);
            void OnDuplicateObjectsBackward(wxCommandEvent& event);
            void OnDuplicateObjectsLeft(wxCommandEvent& event);
            void OnDuplicateObjectsRight(wxCommandEvent& event);
            void OnDuplicateObjectsUp(wxCommandEvent& event);
            void OnDuplicateObjectsDown(wxCommandEvent& event);
            
            void OnRollObjectsCW(wxCommandEvent& event);
            void OnRollObjectsCCW(wxCommandEvent& event);
            void OnPitchObjectsCW(wxCommandEvent& event);
            void OnPitchObjectsCCW(wxCommandEvent& event);
            void OnYawObjectsCW(wxCommandEvent& event);
            void OnYawObjectsCCW(wxCommandEvent& event);
            
            void OnFlipObjectsH(wxCommandEvent& event);
            void OnFlipObjectsV(wxCommandEvent& event);

            void duplicateAndMoveObjects(Math::Direction direction);
            void duplicateObjects();
            void moveObjects(Math::Direction direction);
            Vec3 moveDirection(Math::Direction direction) const;
            void rotateObjects(Math::RotationAxis axis, bool clockwise);
            Vec3 rotationAxis(Math::RotationAxis axis, bool clockwise) const;
            void flipObjects(Math::Direction direction);
        private: // tool mode events
            void OnToggleRotateObjectsTool(wxCommandEvent& event);
            void OnMoveRotationCenterForward(wxCommandEvent& event);
            void OnMoveRotationCenterBackward(wxCommandEvent& event);
            void OnMoveRotationCenterLeft(wxCommandEvent& event);
            void OnMoveRotationCenterRight(wxCommandEvent& event);
            void OnMoveRotationCenterUp(wxCommandEvent& event);
            void OnMoveRotationCenterDown(wxCommandEvent& event);

            void moveRotationCenter(Math::Direction direction);
        private: // other events
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        private: // accelerator table management
            void updateAcceleratorTable(bool hasFocus);
            Action::Context actionContext() const;
        private: // misc
            void flashSelection();
        private: // implement RenderView
            void doInitializeGL();
            void doUpdateViewport(int x, int y, int width, int height);
            bool doShouldRenderFocusIndicator() const;
            void doRender();
            void setupGL(Renderer::RenderContext& renderContext);
            void renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderCompass(Renderer::RenderBatch& renderBatch);
        private: // implement ToolBoxConnector
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
            void doShowPopupMenu();
        private:
            static const GLContextHolder::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView3D__) */
