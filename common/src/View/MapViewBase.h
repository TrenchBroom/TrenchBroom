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

#ifndef __TrenchBroom__MapViewBase__
#define __TrenchBroom__MapViewBase__

#include "Model/ModelTypes.h"
#include "View/ActionContext.h"
#include "View/GLAttribs.h"
#include "View/InputState.h"
#include "MapView.h"
#include "View/RenderView.h"
#include "View/ToolBoxConnector.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class Path;
    }
    
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
        class FlyModeHelper;
        class GLContextManager;
        class MapViewToolBox;
        class MovementRestriction;
        class Selection;
        class Tool;
        
        class MapViewBase : public MapView, public RenderView, public ToolBoxConnector {
        protected:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            MapViewToolBox& m_toolBox;
            
            AnimationManager* m_animationManager;
        private:
            Renderer::MapRenderer& m_renderer;
        protected:
            MapViewBase(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, InputSource inputSource, GLContextManager& contextManager);
        public:
            virtual ~MapViewBase();
        public: // drop targets
            void setToolBoxDropTarget();
            void clearDropTarget();
        private:
            const Renderer::Camera& camera() const;
        private:
            void bindObservers();
            void unbindObservers();
            
            void nodesDidChange(const Model::NodeList& nodes);
            void toolChanged(Tool* tool);
            void commandProcessed(Command* command);
            void selectionDidChange(const Selection& selection);
            void gridDidChange();
            void preferenceDidChange(const IO::Path& path);
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
            
            void OnToggleVertexTool(wxCommandEvent& event);
            
            void OnCancel(wxCommandEvent& event);
            bool cancel();
        private: // other events
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        protected: // accelerator table management
            void updateAcceleratorTable();
        private:
            void updateAcceleratorTable(bool hasFocus);
            ActionContext actionContext() const;
        private: // misc
            void flashSelection();
        private: // implement RenderView interface
            void doInitializeGL(bool firstInitialization);
            bool doShouldRenderFocusIndicator() const;
            void doRender();
            Renderer::RenderContext createRenderContext();
            void setupGL(Renderer::RenderContext& renderContext);
        private: // implement ToolBoxConnector
            void doShowPopupMenu();
        private: // subclassing interface
            virtual Vec3 doGetMoveDirection(Math::Direction direction) const = 0;

            virtual ActionContext doGetActionContext() const = 0;
            virtual wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const = 0;
            virtual bool doCancel() = 0;
            
            virtual Renderer::RenderContext doCreateRenderContext() = 0;
            virtual void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
            virtual void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
            virtual void doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapViewBase__) */
