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

#include "CreateBrushTool.h"

#include "Controller/AddObjectsCommand.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Model/Brush.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Model/Texture.h"
#include "Renderer/BoxInfoRenderer.h"
#include "Renderer/BrushFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/SharedResources.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
#include "View/EditorView.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void CreateBrushTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (dragType() != DTDrag)
                return;
            
            assert(m_brushFigure != NULL);
            m_brushFigure->render(vbo, renderContext);
            
            Renderer::BoxInfoRenderer(m_bounds, document().sharedResources().fontManager()).render(vbo, renderContext);
        }

        void CreateBrushTool::updateBounds(const Vec3f& currentPoint) {
            m_bounds.min = m_bounds.max = m_initialPoint;
            m_bounds.mergeWith(currentPoint);
            
            Utility::Grid& grid = document().grid();
            m_bounds.min = grid.snapDown(m_bounds.min);
            m_bounds.max = grid.snapUp(m_bounds.max);
            
            for (size_t i = 0; i < 3; i++)
                if (m_bounds.max[i] <= m_bounds.min[i])
                    m_bounds.max[i] = m_bounds.min[i] + grid.actualSize();
        }

        void CreateBrushTool::handleModifierKeyChange(InputState& inputState) {
            if (dragType() != DTDrag)
                return;

            resetPlane(inputState);
        }

        bool CreateBrushTool::handleStartPlaneDrag(InputState& inputState, Planef& plane, Vec3f& initialPoint) {
            assert(m_brush == NULL);
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone ||
                editStateManager.selectionMode() != Model::EditStateManager::SMNone)
                return false;
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if (hit != NULL) {
                initialPoint = hit->hitPoint();
            } else {
                Renderer::Camera& camera = view().camera();
                initialPoint = camera.defaultPoint(inputState.pickRay().direction);
            }

            plane = Planef(Vec3f::PosZ, initialPoint);
            m_initialPoint = initialPoint;
            updateBounds(m_initialPoint);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::TextureRendererManager& textureRendererManager = document().sharedResources().textureRendererManager();

            m_brushFigure = new Renderer::BrushFigure(textureRendererManager);
            m_brushFigure->setFaceColor(prefs.getColor(Preferences::FaceColor));
            m_brushFigure->setEdgeColor(prefs.getColor(Preferences::SelectedEdgeColor));
            m_brushFigure->setOccludedEdgeColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
            m_brushFigure->setEdgeMode(Renderer::BrushFigure::EMRenderOccluded);

            m_brush = new Model::Brush(document().map().worldBounds(), document().map().forceIntegerFacePoints(), m_bounds, document().mruTexture());
            m_brushFigure->setBrush(*m_brush);
            
            return true;
        }
        
        void CreateBrushTool::handleResetPlane(InputState& inputState, Planef& plane, Vec3f& initialPoint) {
            float distance = plane.intersectWithRay(inputState.pickRay());
            if (Math<float>::isnan(distance))
                return;
            initialPoint = inputState.pickRay().pointAtDistance(distance);
            
            if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                Vec3f planeNorm = inputState.pickRay().direction;
                planeNorm[2] = 0.0f;
                planeNorm.normalize();
                plane = Planef(planeNorm, initialPoint);
            } else {
                plane = Planef::horizontalDragPlane(initialPoint);
            }
        }

        bool CreateBrushTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            assert(m_brush != NULL);
            assert(m_brushFigure != NULL);
            
            updateBounds(curPoint);
            
            delete m_brush;
            m_brush = new Model::Brush(document().map().worldBounds(), document().map().forceIntegerFacePoints(), m_bounds, document().mruTexture());
            m_brushFigure->setBrush(*m_brush);
            return true;
        }
        
        void CreateBrushTool::handleEndPlaneDrag(InputState& inputState) {
            assert(m_brush != NULL);
            assert(m_brushFigure != NULL);
            
            Controller::AddObjectsCommand* addBrushCommand = Controller::AddObjectsCommand::addBrush(document(), *m_brush);
            Controller::ChangeEditStateCommand* selectBrushCommand = Controller::ChangeEditStateCommand::replace(document(), *m_brush);
            
            beginCommandGroup(wxT("Create Brush"));
            submitCommand(addBrushCommand);
            submitCommand(selectBrushCommand);
            endCommandGroup();
            
            m_brush = NULL;

            deleteFigure(m_brushFigure);
            m_brushFigure = NULL;
        }

        CreateBrushTool::CreateBrushTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        PlaneDragTool(documentViewHolder, inputController, true),
        m_filter(Model::VisibleFilter(documentViewHolder.view().filter())),
        m_boundsChanged(false),
        m_brush(NULL),
        m_brushFigure(NULL) {}
    }
}
