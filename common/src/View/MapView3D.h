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

#ifndef TrenchBroom_MapView3D
#define TrenchBroom_MapView3D

#include "MathUtils.h"
#include "Model/ModelTypes.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/Action.h"
#include "View/MapViewBase.h"
#include "View/MovementRestriction.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class MapRenderer;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class CameraTool3D;
        class ClipToolAdapter3D;
        class CreateComplexBrushToolAdapter3D;
        class CreateEntityToolAdapter;
        class CreateSimpleBrushToolAdapter3D;
        class FlyModeHelper;
        class GLContextManager;
        class MoveObjectsToolAdapter;
        class ResizeBrushesToolAdapter;
        class RotateObjectsToolAdapter;
        class SetBrushFaceAttributesTool;
        class VertexToolAdapter;
        
        class MapView3D : public MapViewBase {
        private:
            MovementRestriction m_movementRestriction;
            Renderer::PerspectiveCamera m_camera;
            
            ClipToolAdapter3D* m_clipToolAdapter;
            CreateComplexBrushToolAdapter3D* m_createComplexBrushToolAdapter;
            CreateEntityToolAdapter* m_createEntityToolAdapter;
            CreateSimpleBrushToolAdapter3D* m_createSimpleBrushToolAdapter;
            MoveObjectsToolAdapter* m_moveObjectsToolAdapter;
            ResizeBrushesToolAdapter* m_resizeBrushesToolAdapter;
            RotateObjectsToolAdapter* m_rotateObjectsToolAdapter;
            SetBrushFaceAttributesTool* m_setBrushFaceAttributesTool;
            VertexToolAdapter* m_vertexToolAdapter;
            CameraTool3D* m_cameraTool;
            
            FlyModeHelper* m_flyModeHelper;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager);
            ~MapView3D();
        private:
            void initializeCamera();
            void initializeToolChain(MapViewToolBox& toolBox);
            void destroyToolChain();
        public: // camera control
            bool cameraFlyModeActive() const;
            void toggleCameraFlyMode();
        private: // notification
            void bindObservers();
            void unbindObservers();
            void cameraDidChange(const Renderer::Camera* camera);
        private: // interaction events
            void bindEvents();
            
            void OnKeyDown(wxKeyEvent& event);
            void OnKeyUp(wxKeyEvent& event);
            void key(wxKeyEvent& event);

            void OnMouseMotion(wxMouseEvent& event);
            
            void OnToggleMovementRestriction(wxCommandEvent& event);
            void OnSetMovementRestrictionX(wxCommandEvent& event);
            void OnSetMovementRestrictionY(wxCommandEvent& event);
            void OnSetMovementRestrictionZ(wxCommandEvent& event);
            
            void OnPerformCreateBrush(wxCommandEvent& event);

            void OnMoveTexturesUp(wxCommandEvent& event);
            void OnMoveTexturesDown(wxCommandEvent& event);
            void OnMoveTexturesLeft(wxCommandEvent& event);
            void OnMoveTexturesRight(wxCommandEvent& event);
            void OnRotateTexturesCW(wxCommandEvent& event);
            void OnRotateTexturesCCW(wxCommandEvent& event);

            float moveTextureDistance() const;
            void moveTextures(const Vec2f& offset);
            float rotateTextureAngle(bool clockwise) const;
            void rotateTextures(float angle);
        private: // tool mode events
            void OnToggleFlyMode(wxCommandEvent& event);
        private: // other events
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        private:
            void updateVerticalMovementRestriction(const wxKeyboardState& state);
        private: // implement ToolBoxConnector interface
            PickRequest doGetPickRequest(int x, int y) const;
            Model::PickResult doPick(const Ray3& pickRay) const;
        private: // implement RenderView interface
            void doUpdateViewport(int x, int y, int width, int height);
        private: // implement MapView interface
            Vec3 doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const;

            bool doCanSelectTall();
            void doSelectTall();

            void doFocusCameraOnSelection();
            
            class ComputeCameraCenterPositionVisitor;
            class ComputeCameraCenterOffsetVisitor;
            Vec3f focusCameraOnObjectsPosition(const Model::NodeList& nodes);

            void doMoveCameraToPosition(const Vec3& position);
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration = DefaultCameraAnimationDuration);
            
            void doMoveCameraToCurrentTracePoint();
        private: // implement MapViewBase interface
            Vec3 doGetMoveDirection(Math::Direction direction) const;
            Vec3 doComputePointEntityPosition(const BBox3& bounds) const;
            
            ActionContext doGetActionContext() const;
            wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const;
            bool doCancel();
            
            Renderer::RenderContext doCreateRenderContext();
            void doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            void doAfterPopupMenu();
        private: // implement CameraLinkableView interface
            void doLinkCamera(CameraLinkHelper& linkHelper);
        };
    }
}

#endif /* defined(TrenchBroom_MapView3D) */
