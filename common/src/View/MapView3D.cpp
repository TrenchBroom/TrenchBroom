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

#include "MapView3D.h"
#include "Logger.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CommandIds.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer) :
        RenderView(parent, attribs()),
        ToolBoxConnector(this, toolBox),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager()),
        m_vbo(new Renderer::Vbo(0xFFFFFFF)),
        m_renderer(renderer),
        m_camera(),
        m_compass(new Renderer::Compass(toolBox.movementRestriction())),
        m_flyModeHelper(new FlyModeHelper(this, m_camera)) {
            bindObservers();
            bindEvents();
            updateAcceleratorTable(HasFocus());
        }

        MapView3D::~MapView3D() {
            unbindObservers();
            delete m_flyModeHelper;
            m_animationManager->Delete();
            delete m_compass;
            delete m_vbo;
        }
        
        Renderer::Camera* MapView3D::camera() {
            return &m_camera;
        }
        
        bool MapView3D::cameraFlyModeActive() const {
            return m_flyModeHelper->enabled();
        }
        
        void MapView3D::toggleCameraFlyMode() {
            if (!cameraFlyModeActive()) {
                m_toolBox.disable();
                m_flyModeHelper->enable();
            } else {
                m_flyModeHelper->disable();
                m_toolBox.enable();
            }
            updateAcceleratorTable(HasFocus());
            Refresh();
        }

        void MapView3D::centerCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::EntityList& entities = document->selectedNodes().entities();
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            assert(!entities.empty() || !brushes.empty());
            
            const Vec3 newPosition = centerCameraOnObjectsPosition(entities, brushes);
            moveCameraToPosition(newPosition);
        }

        void MapView3D::moveCameraToPosition(const Vec3& position) {
            animateCamera(position, m_camera.direction(), m_camera.up());
        }
        
        void MapView3D::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }

        
        Vec3f MapView3D::centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::BrushList::const_iterator brushIt, brushEnd;
            
            float minDist = std::numeric_limits<float>::max();
            Vec3 center;
            size_t count = 0;
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (!entity->hasChildren()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        const Vec3f toPosition = vertex - m_camera.position();
                        minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                        center += vertices[i];
                        ++count;
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    const Vec3f toPosition = Vec3f(vertex->position) - m_camera.position();
                    minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                    center += vertex->position;
                    ++count;
                }
            }
            
            center /= static_cast<FloatType>(count);
            
            // act as if the camera were there already:
            const Vec3f oldPosition = m_camera.position();
            m_camera.moveTo(Vec3f(center));
            
            float offset = std::numeric_limits<float>::max();
            
            Plane3f frustumPlanes[4];
            m_camera.frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (!entity->hasChildren()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        
                        for (size_t j = 0; j < 4; ++j) {
                            const Plane3f& plane = frustumPlanes[j];
                            const float dist = (vertex - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                            offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                        }
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    
                    for (size_t j = 0; j < 4; ++j) {
                        const Plane3f& plane = frustumPlanes[j];
                        const float dist = (Vec3f(vertex->position) - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                        offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                    }
                }
            }
            
            // jump back
            m_camera.moveTo(oldPosition);
            
            return center + m_camera.direction() * offset;
        }
        
        void MapView3D::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->commandDoneNotifier.addObserver(this, &MapView3D::commandProcessed);
            document->commandUndoneNotifier.addObserver(this, &MapView3D::commandProcessed);
            document->selectionDidChangeNotifier.addObserver(this, &MapView3D::selectionDidChange);
            
            Grid& grid = document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapView3D::gridDidChange);
            
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView3D::cameraDidChange);
            m_toolBox.toolActivatedNotifier.addObserver(this, &MapView3D::toolChanged);
        }
        
        void MapView3D::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->commandDoneNotifier.removeObserver(this, &MapView3D::commandProcessed);
                document->commandUndoneNotifier.removeObserver(this, &MapView3D::commandProcessed);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView3D::selectionDidChange);

                Grid& grid = document->grid();
                grid.gridDidChangeNotifier.removeObserver(this, &MapView3D::gridDidChange);
            }

            // toolbox has already been deleted at this point
            // m_toolBox.toolActivatedNotifier.removeObserver(this, &MapView3D::toolChanged);

            // camera has already been deleted at this point
            // m_camera.cameraDidChangeNotifier.addObserver(this, &MapView3D::cameraDidChange);
        }
        
        void MapView3D::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }

        void MapView3D::toolChanged(Tool* tool) {
            updateHits();
            updateAcceleratorTable(HasFocus());
            Refresh();
        }

        void MapView3D::commandProcessed(Command* command) {
            updateHits();
            Refresh();
        }

        void MapView3D::selectionDidChange(const Selection& selection) {
            updateAcceleratorTable(HasFocus());
        }

        void MapView3D::gridDidChange() {
            Refresh();
        }

        void MapView3D::bindEvents() {
            /*
            Bind(wxEVT_KEY_DOWN, &MapView3D::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView3D::OnKey, this);
            */
            
            Bind(wxEVT_SET_FOCUS, &MapView3D::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView3D::OnKillFocus, this);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnToggleClipTool,               this, CommandIds::Actions::ToggleClipTool);
            Bind(wxEVT_MENU, &MapView3D::OnToggleClipSide,               this, CommandIds::Actions::ToggleClipSide);
            Bind(wxEVT_MENU, &MapView3D::OnPerformClip,                  this, CommandIds::Actions::PerformClip);
            Bind(wxEVT_MENU, &MapView3D::OnDeleteLastClipPoint,          this, CommandIds::Actions::DeleteLastClipPoint);
            */
            Bind(wxEVT_MENU, &MapView3D::OnToggleVertexTool,             this, CommandIds::Actions::ToggleVertexTool);
            /*
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesForward,          this, CommandIds::Actions::MoveVerticesForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesBackward,         this, CommandIds::Actions::MoveVerticesBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesLeft,             this, CommandIds::Actions::MoveVerticesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesRight,            this, CommandIds::Actions::MoveVerticesRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesUp,               this, CommandIds::Actions::MoveVerticesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesDown,             this, CommandIds::Actions::MoveVerticesDown);
            */
             
            /*
            Bind(wxEVT_MENU, &MapView3D::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
             */
            
            Bind(wxEVT_MENU, &MapView3D::OnDeleteObjects,                this, CommandIds::Actions::DeleteObjects);
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsForward,           this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsBackward,          this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsLeft,              this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsRight,             this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsUp,                this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsDown,              this, CommandIds::Actions::MoveObjectsDown);
            
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjects,             this, CommandIds::Actions::DuplicateObjects);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsForward,      this, CommandIds::Actions::DuplicateObjectsForward);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsBackward,     this, CommandIds::Actions::DuplicateObjectsBackward);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsLeft,         this, CommandIds::Actions::DuplicateObjectsLeft);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsRight,        this, CommandIds::Actions::DuplicateObjectsRight);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsUp,           this, CommandIds::Actions::DuplicateObjectsUp);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsDown,         this, CommandIds::Actions::DuplicateObjectsDown);
            
            Bind(wxEVT_MENU, &MapView3D::OnRollObjectsCW,                this, CommandIds::Actions::RollObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnRollObjectsCCW,               this, CommandIds::Actions::RollObjectsCCW);
            Bind(wxEVT_MENU, &MapView3D::OnPitchObjectsCW,               this, CommandIds::Actions::PitchObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnPitchObjectsCCW,              this, CommandIds::Actions::PitchObjectsCCW);
            Bind(wxEVT_MENU, &MapView3D::OnYawObjectsCW,                 this, CommandIds::Actions::YawObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnYawObjectsCCW,                this, CommandIds::Actions::YawObjectsCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnFlipObjectsH,                 this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_MENU, &MapView3D::OnFlipObjectsV,                 this, CommandIds::Actions::FlipObjectsVertically);
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesUp,               this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesDown,             this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesLeft,             this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesRight,            this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCW,             this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCCW,            this, CommandIds::Actions::RotateTexturesCCW);

            Bind(wxEVT_MENU, &MapView3D::OnToggleRotateObjectsTool,      this, CommandIds::Actions::ToggleRotateObjectsTool);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterForward,    this, CommandIds::Actions::MoveRotationCenterForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterBackward,   this, CommandIds::Actions::MoveRotationCenterBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterLeft,       this, CommandIds::Actions::MoveRotationCenterLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterRight,      this, CommandIds::Actions::MoveRotationCenterRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterUp,         this, CommandIds::Actions::MoveRotationCenterUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterDown,       this, CommandIds::Actions::MoveRotationCenterDown);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleFlyMode,                this, CommandIds::Actions::ToggleFlyMode);
            Bind(wxEVT_MENU, &MapView3D::OnCancel,                       this, CommandIds::Actions::Cancel);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnPopupReparentBrushes,         this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_MENU, &MapView3D::OnPopupMoveBrushesToWorld,      this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreatePointEntity,       this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreateBrushEntity,       this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            */
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView3D::OnActivateFrame, this);
        }
        
        void MapView3D::OnDeleteObjects(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                document->deleteObjects();
        }

        void MapView3D::OnMoveObjectsForward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Forward);
        }
        
        void MapView3D::OnMoveObjectsBackward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Backward);
        }
        
        void MapView3D::OnMoveObjectsLeft(wxCommandEvent& event) {
            moveObjects(Math::Direction_Left);
        }
        
        void MapView3D::OnMoveObjectsRight(wxCommandEvent& event) {
            moveObjects(Math::Direction_Right);
        }
        
        void MapView3D::OnMoveObjectsUp(wxCommandEvent& event) {
            moveObjects(Math::Direction_Up);
        }
        
        void MapView3D::OnMoveObjectsDown(wxCommandEvent& event) {
            moveObjects(Math::Direction_Down);
        }
        
        void MapView3D::OnDuplicateObjects(wxCommandEvent& event) {
            duplicateObjects();
        }

        void MapView3D::OnDuplicateObjectsForward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Forward);
        }
        
        void MapView3D::OnDuplicateObjectsBackward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Backward);
        }
        
        void MapView3D::OnDuplicateObjectsLeft(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Left);
        }
        
        void MapView3D::OnDuplicateObjectsRight(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Right);
        }
        
        void MapView3D::OnDuplicateObjectsUp(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Up);
        }
        
        void MapView3D::OnDuplicateObjectsDown(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Down);
        }

        void MapView3D::OnRollObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Roll, true);
        }
        
        void MapView3D::OnRollObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Roll, false);
        }
        
        void MapView3D::OnPitchObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Pitch, true);
        }
        
        void MapView3D::OnPitchObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Pitch, false);
        }
        
        void MapView3D::OnYawObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Yaw, true);
        }
        
        void MapView3D::OnYawObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Yaw, false);
        }
        
        void MapView3D::OnFlipObjectsH(wxCommandEvent& event) {
            flipObjects(Math::Direction_Left);
        }
        
        void MapView3D::OnFlipObjectsV(wxCommandEvent& event) {
            flipObjects(Math::Direction_Up);
        }
        
        void MapView3D::duplicateAndMoveObjects(const Math::Direction direction) {
            Transaction transaction(m_document);
            duplicateObjects();
            moveObjects(direction);
        }
        
        void MapView3D::duplicateObjects() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            document->duplicateObjects();
            flashSelection();
        }

        void MapView3D::moveObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        Vec3 MapView3D::moveDirection(const Math::Direction direction) const {
            switch (direction) {
                case Math::Direction_Forward: {
                    Vec3 dir = m_camera.direction().firstAxis();
                    if (dir.z() < 0.0)
                        dir = m_camera.up().firstAxis();
                    else if (dir.z() > 0.0)
                        dir = -m_camera.up().firstAxis();
                    return dir;
                }
                case Math::Direction_Backward:
                    return -moveDirection(Math::Direction_Forward);
                case Math::Direction_Left:
                    return -moveDirection(Math::Direction_Right);
                case Math::Direction_Right: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == moveDirection(Math::Direction_Forward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::Direction_Up:
                    return Vec3::PosZ;
                case Math::Direction_Down:
                    return Vec3::NegZ;
                    DEFAULT_SWITCH()
            }
        }

        void MapView3D::rotateObjects(const Math::RotationAxis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Vec3 axis = rotationAxis(axisSpec, clockwise);
            const double angle = m_toolBox.rotateObjectsToolActive() ? std::abs(m_toolBox.rotateToolAngle()) : Math::C::piOverTwo();
            
            const Grid& grid = document->grid();
            const Vec3 center = m_toolBox.rotateObjectsToolActive() ? m_toolBox.rotateToolCenter() : grid.referencePoint(document->selectionBounds());
            
            document->rotateObjects(center, axis, angle);
        }
        
        Vec3 MapView3D::rotationAxis(const Math::RotationAxis axisSpec, const bool clockwise) const {
            Vec3 axis;
            switch (axisSpec) {
                case Math::RotationAxis_Roll:
                    axis = -moveDirection(Math::Direction_Forward);
                    break;
                case Math::RotationAxis_Pitch:
                    axis = moveDirection(Math::Direction_Right);
                    break;
                case Math::RotationAxis_Yaw:
                    axis = Vec3::PosZ;
                    break;
                    DEFAULT_SWITCH()
            }
            
            if (clockwise)
                axis = -axis;
            return axis;
        }
        
        void MapView3D::flipObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            const Math::Axis::Type axis = moveDirection(direction).firstComponent();
            
            document->flipObjects(center, axis);
        }

        void MapView3D::OnMoveTexturesUp(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesDown(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, -moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesLeft(wxCommandEvent& event) {
            moveTextures(Vec2f(-moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnMoveTexturesRight(wxCommandEvent& event) {
            moveTextures(Vec2f(moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(true));
        }
        
        void MapView3D::OnRotateTexturesCCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(false));
        }
        
        float MapView3D::moveTextureDistance() const {
            const Grid& grid = lock(m_document)->grid();
            const float gridSize = static_cast<float>(grid.actualSize());
            
            const wxMouseState mouseState = wxGetMouseState();
            switch (mouseState.GetModifiers()) {
                case wxMOD_CMD:
                    return 1.0f;
                case wxMOD_SHIFT:
                    return 2.0f * gridSize;
                default:
                    return gridSize;
            }
        }
        
        void MapView3D::moveTextures(const Vec2f& offset) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces())
                document->moveTextures(m_camera.up(), m_camera.right(), offset);
        }
        
        float MapView3D::rotateTextureAngle(const bool clockwise) const {
            const Grid& grid = lock(m_document)->grid();
            const float gridAngle = static_cast<float>(Math::degrees(grid.angle()));
            float angle = 0.0f;
            
            const wxMouseState mouseState = wxGetMouseState();
            switch (mouseState.GetModifiers()) {
                case wxMOD_CMD:
                    angle = 1.0f;
                    break;
                case wxMOD_SHIFT:
                    angle = 90.0f;
                    break;
                default:
                    angle = gridAngle;
                    break;
            }
            
            return clockwise ? angle : -angle;
        }
        
        void MapView3D::rotateTextures(const float angle) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces())
                document->rotateTextures(angle);
        }

        void MapView3D::OnToggleRotateObjectsTool(wxCommandEvent& event) {
            m_toolBox.toggleRotateObjectsTool();
        }

        void MapView3D::OnMoveRotationCenterForward(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Forward);
        }
        
        void MapView3D::OnMoveRotationCenterBackward(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Backward);
        }
        
        void MapView3D::OnMoveRotationCenterLeft(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Left);
        }
        
        void MapView3D::OnMoveRotationCenterRight(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Right);
        }
        
        void MapView3D::OnMoveRotationCenterUp(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Up);
        }
        
        void MapView3D::OnMoveRotationCenterDown(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Down);
        }
        
        void MapView3D::moveRotationCenter(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            Refresh();
        }

        void MapView3D::OnToggleVertexTool(wxCommandEvent& event) {
            m_toolBox.toggleVertexTool();
        }

        void MapView3D::OnToggleFlyMode(wxCommandEvent& event) {
            toggleCameraFlyMode();
        }

        void MapView3D::OnCancel(wxCommandEvent& event) {
            if (cameraFlyModeActive()) {
                toggleCameraFlyMode();
                return;
            }
            
            if (m_toolBox.cancel())
                return;
            
            lock(m_document)->deselectAll();
        }

        void MapView3D::OnSetFocus(wxFocusEvent& event) {
            updateAcceleratorTable(true);
            event.Skip();
        }
        
        void MapView3D::OnKillFocus(wxFocusEvent& event) {
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            updateAcceleratorTable(false);
            event.Skip();
        }

        void MapView3D::OnActivateFrame(wxActivateEvent& event) {
            if (event.GetActive())
                updateLastActivation();
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            event.Skip();
        }

        void MapView3D::updateAcceleratorTable(const bool hasFocus) {
            if (hasFocus) {
                const ActionManager& actionManager = ActionManager::instance();
                const Action::Context context = actionContext();
                const wxAcceleratorTable acceleratorTable = actionManager.createMapViewAcceleratorTable(context);
                SetAcceleratorTable(acceleratorTable);
            } else {
                SetAcceleratorTable(wxNullAcceleratorTable);
            }
        }
        
        Action::Context MapView3D::actionContext() const {
            /*
            if (clipToolActive())
                return Action::Context_ClipTool;
             */
            if (m_toolBox.vertexToolActive())
                return Action::Context_VertexTool;
            if (m_toolBox.rotateObjectsToolActive())
                return Action::Context_RotateTool;
            if (cameraFlyModeActive())
                return Action::Context_FlyMode;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                return Action::Context_NodeSelection;
            if (document->hasSelectedBrushFaces())
                return Action::Context_FaceSelection;
            return Action::Context_Default;
        }

        void MapView3D::flashSelection() {
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, *this, 180);
            m_animationManager->runAnimation(animation, true);
        }

        void MapView3D::doInitializeGL() {
            const wxString vendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            const wxString renderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            const wxString version  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
            
            m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", renderer, version, vendor));
            m_logger->info("Depth buffer bits: %d", depthBits());
            
            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
        }
        
        void MapView3D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }
        
        bool MapView3D::doShouldRenderFocusIndicator() const {
            return true;
        }
        
        void MapView3D::doRender() {
            Renderer::RenderContext renderContext = createRenderContext();
            
            setupGL(renderContext);
            setRenderOptions(renderContext);
            
            Renderer::RenderBatch renderBatch(*m_vbo);
            
            renderMap(renderContext, renderBatch);
            renderTools(renderContext, renderBatch);
            renderCompass(renderBatch);
            
            renderBatch.render(renderContext);
        }

        Renderer::RenderContext MapView3D::createRenderContext() {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            
            Renderer::RenderContext renderContext(m_camera, contextHolder()->fontManager(), contextHolder()->shaderManager());
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());
            return renderContext;
        }

        void MapView3D::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }

        void MapView3D::renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_renderer.render(renderContext, renderBatch);
        }

        void MapView3D::renderCompass(Renderer::RenderBatch& renderBatch) {
            m_compass->render(renderBatch);
        }

        Ray3 MapView3D::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits MapView3D::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }
        
        void MapView3D::doShowPopupMenu() {
        }

        const GLContextHolder::GLAttribs& MapView3D::attribs() {
            static bool initialized = false;
            static GLContextHolder::GLAttribs attribs;
            if (initialized)
                return attribs;
            
            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };
            
            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }
            
            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }
        
        int MapView3D::depthBits() {
            return attribs()[3];
        }
        
        bool MapView3D::multisample() {
            return attribs()[4] != 0;
        }
    }
}
