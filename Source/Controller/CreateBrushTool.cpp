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
#include "Renderer/BrushFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/SharedResources.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
#include "View/EditorView.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool CreateBrushTool::handleUpdateState(InputState& inputState) {
            if (dragType() != DTDrag)
                return false;
            
            bool update = m_boundsChanged;
            m_boundsChanged = false;
            return update;
        }
        
        void CreateBrushTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (dragType() != DTDrag)
                return;
            
            assert(m_brushFigure != NULL);
            m_brushFigure->render(vbo, renderContext);
        }

        void CreateBrushTool::updateBounds(const Vec3f& currentPoint) {
            m_bounds.min = m_bounds.max = m_initialPoint;
            m_bounds.mergeWith(currentPoint);
            
            Utility::Grid& grid = document().grid();
            m_bounds.min = grid.snapDown(m_bounds.min);
            m_bounds.max = grid.snapUp(m_bounds.max);
            m_boundsChanged = true;
        }

        bool CreateBrushTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            assert(m_brush == NULL);
            
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Utility::Grid& grid = document().grid();

            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            if (hit != NULL) {
                const Vec3f& normal = hit->face().boundary().normal.firstAxis();
                initialPoint = hit->hitPoint();
                plane = Plane(normal, initialPoint + normal * grid.actualSize());
            } else {
                Renderer::Camera& camera = view().camera();
                const Vec3f normal = -camera.direction().firstAxis();
                initialPoint = camera.defaultPoint(inputState.pickRay().direction);
                plane = Plane(normal, initialPoint + normal * grid.actualSize());
            }
            
            m_initialPoint = initialPoint;
            updateBounds(m_initialPoint);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::TextureRendererManager& textureRendererManager = document().sharedResources().textureRendererManager();
            const Color& faceColor = prefs.getColor(Preferences::FaceColor);
            const Color& edgeColor = prefs.getColor(Preferences::SelectedEdgeColor);
            m_brushFigure = new Renderer::BrushFigure(textureRendererManager, faceColor, edgeColor, true);

            m_brush = new Model::Brush(document().map().worldBounds(), m_bounds, document().mruTexture());
            m_brushFigure->setBrush(*m_brush);
            
            return true;
        }
        
        void CreateBrushTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            assert(m_brush != NULL);
            assert(m_brushFigure != NULL);
            
            updateBounds(curPoint);
            
            delete m_brush;
            m_brush = new Model::Brush(document().map().worldBounds(), m_bounds, document().mruTexture());
            m_brushFigure->setBrush(*m_brush);
        }
        
        void CreateBrushTool::handleEndPlaneDrag(InputState& inputState) {
            assert(m_brush != NULL);
            assert(m_brushFigure != NULL);
            
            Controller::AddObjectsCommand* addBrushCommand = Controller::AddObjectsCommand::addBrush(document(), *m_brush);
            Controller::ChangeEditStateCommand* selectBrushCommand = Controller::ChangeEditStateCommand::select(document(), *m_brush);
            
            beginCommandGroup(wxT("Create Brush"));
            submitCommand(addBrushCommand);
            submitCommand(selectBrushCommand);
            endCommandGroup();
            
            m_brush = NULL;
            
            delete m_brushFigure;
            m_brushFigure = NULL;
        }

        CreateBrushTool::CreateBrushTool(View::DocumentViewHolder& documentViewHolder) :
        PlaneDragTool(documentViewHolder, true),
        m_boundsChanged(false),
        m_brush(NULL),
        m_brushFigure(NULL) {}
    }
}