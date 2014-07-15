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

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__


#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Notifier.h"
#include "Assets/AssetTypes.h"
#include "Controller/Command.h"
#include "Renderer/BoundsGuideRenderer.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/Vbo.h"
#include "View/Action.h"
#include "View/FlyModeHelper.h"
#include "View/GLContextHolder.h"
#include "View/MovementRestriction.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ViewTypes.h"

#include <wx/datetime.h>
#include <wx/longlong.h>

#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }
    
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class AnimationManager;
        class Tool;
        class CameraTool;
        class ClipTool;
        class CreateBrushTool;
        class CreateEntityTool;
        class MoveObjectsTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class SelectionTool;
        class SetFaceAttribsTool;
        class TextureTool;
        class VertexTool;
        
        class MapView : public RenderView, public ToolBoxHelper {
        private:
            static const int FlyTimerId;
            
            Logger* m_logger;
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            Renderer::Camera& m_camera;
            
            MovementRestriction m_movementRestriction;
            AnimationManager* m_animationManager;
            
            ToolBox m_toolBox;
            CameraTool* m_cameraTool;
            ClipTool* m_clipTool;
            CreateBrushTool* m_createBrushTool;
            CreateEntityTool* m_createEntityTool;
            MoveObjectsTool* m_moveObjectsTool;
            VertexTool* m_vertexTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
            SetFaceAttribsTool* m_setFaceAttribsTool;
            TextureTool* m_textureTool;

            FlyModeHelper m_flyModeHelper;
            
            Renderer::MapRenderer m_renderer;
            Renderer::Compass m_compass;
            Renderer::BoundsGuideRenderer m_selectionGuide;
        public:
            MapView(wxWindow* parent, Logger* logger, View::MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera);
            ~MapView();
            
            void setToolboxDropTarget();
            void clearToolboxDropTarget();
            
            void centerCameraOnSelection();
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration);
            
            bool cameraFlyModeActive() const;
            void toggleCameraFlyMode();
            
            void toggleMovementRestriction();

            bool anyToolActive() const;
            void toggleClipTool();
            bool clipToolActive() const;
            bool canToggleClipSide() const;
            void toggleClipSide();
            bool canPerformClip() const;
            void performClip();
            bool canDeleteLastClipPoint() const;
            void deleteLastClipPoint();
            
            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            
            void toggleVertexTool();
            bool vertexToolActive() const;
            bool hasSelectedVertices() const;
            bool canSnapVertices() const;
            void snapVertices(size_t snapTo);
            
            void toggleTextureTool();
            bool textureToolActive() const;
            
            Vec3 pasteObjectsDelta(const BBox3& bounds) const;

            void OnToggleClipTool(wxCommandEvent& event);
            void OnToggleClipSide(wxCommandEvent& event);
            void OnPerformClip(wxCommandEvent& event);
            void OnDeleteLastClipPoint(wxCommandEvent& event);

            void OnToggleVertexTool(wxCommandEvent& event);
            void OnMoveVerticesForward(wxCommandEvent& event);
            void OnMoveVerticesBackward(wxCommandEvent& event);
            void OnMoveVerticesLeft(wxCommandEvent& event);
            void OnMoveVerticesRight(wxCommandEvent& event);
            void OnMoveVerticesUp(wxCommandEvent& event);
            void OnMoveVerticesDown(wxCommandEvent& event);
        private:
            void moveVertices(Math::Direction direction);
        public:
            void OnToggleRotateObjectsTool(wxCommandEvent& event);

            void OnToggleFlyMode(wxCommandEvent& event);
            
            void OnToggleMovementRestriction(wxCommandEvent& event);

            void OnDeleteObjects(wxCommandEvent& event);
            
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
            
            void OnDuplicateObjectsForward(wxCommandEvent& event);
            void OnDuplicateObjectsBackward(wxCommandEvent& event);
            void OnDuplicateObjectsLeft(wxCommandEvent& event);
            void OnDuplicateObjectsRight(wxCommandEvent& event);
            void OnDuplicateObjectsUp(wxCommandEvent& event);
            void OnDuplicateObjectsDown(wxCommandEvent& event);
        private:
            void rotateObjects(RotationAxis axis, bool clockwise);
            void flipObjects(Math::Direction direction);
            
            void duplicateAndMoveObjects(Math::Direction direction);
            void duplicateObjects();
            void moveObjects(Math::Direction direction);
        public:
            void OnMoveTexturesUp(wxCommandEvent& event);
            void OnMoveTexturesDown(wxCommandEvent& event);
            void OnMoveTexturesLeft(wxCommandEvent& event);
            void OnMoveTexturesRight(wxCommandEvent& event);
            void OnRotateTexturesCW(wxCommandEvent& event);
            void OnRotateTexturesCCW(wxCommandEvent& event);
        private:
            float moveTextureDistance() const;
            void moveTextures(const Vec2f& offset);
            float rotateTextureAngle(bool clockwise) const;
            void rotateTextures(float angle);
        public:
            void OnKey(wxKeyEvent& event);

            void OnActivateFrame(wxActivateEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnPopupReparentBrushes(wxCommandEvent& event);
            void OnPopupMoveBrushesToWorld(wxCommandEvent& event);
            void OnPopupCreatePointEntity(wxCommandEvent& event);
            void OnPopupCreateBrushEntity(wxCommandEvent& event);
            void OnUpdatePopupMenuItem(wxUpdateUIEvent& event);
        private:
            void updateAcceleratorTable();
            Action::Context actionContext() const;
            
            void updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const;
            void updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const;
            Model::Entity* findNewBrushParent(const Model::BrushList& brushes) const;
            void reparentBrushes(const Model::BrushList& brushes, Model::Entity* newParent);
            bool canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const;
            Model::BrushList filterReparentableBrushes(const Model::BrushList& brushes, Model::Entity* newParent);
            
            Assets::EntityDefinition* findEntityDefinition(const Assets::EntityDefinitionGroups& groups, const size_t index) const;
            void createPointEntity(const Assets::PointEntityDefinition& definition);
            void createBrushEntity(const Assets::BrushEntityDefinition& definition);
            
            Vec3 moveDirection(Math::Direction direction) const;
            Vec3f centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes);
            
            void resetCamera();

            void bindObservers();
            void unbindObservers();
            
            void documentWasNewedOrLoaded();
            void objectWasAddedOrRemoved(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            void gridDidChange();
            void commandDoneOrUndone(Controller::Command::Ptr command);
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);
            void cameraDidChange(const Renderer::Camera* camera);
            
            void createTools();
            void deleteTools();

            void doUpdateViewport(int x, int y, int width, int height);
            void doInitializeGL();

            void doRender();
            void setupGL(Renderer::RenderContext& context);
            void setRenderOptions(Renderer::RenderContext& context);
            void renderCoordinateSystem(Renderer::RenderContext& context);
            void renderCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor);
            void renderMap(Renderer::RenderContext& context);
            void renderSelectionGuide(Renderer::RenderContext& context);
            void renderToolBox(Renderer::RenderContext& context);
            void renderCompass(Renderer::RenderContext& context);
            
            Ray3 doGetPickRay(int x, int y) const;
            Hits doPick(const Ray3& pickRay) const;
            void doShowPopupMenu();
            wxMenu* makeEntityGroupsMenu(const Assets::EntityDefinitionGroups& groups, int id);

            void bindEvents();

            static const GLContextHolder::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
            static Renderer::TextureFont& defaultFont(Renderer::FontManager& fontManager);
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
