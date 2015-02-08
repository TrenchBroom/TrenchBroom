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

#include "CreateBrushToolAdapter2D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateBrushToolAdapter2D::CreateBrushToolAdapter2D(CreateBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(m_tool != NULL);
        }

        CreateBrushToolAdapter2D::~CreateBrushToolAdapter2D() {}
        
        Tool* CreateBrushToolAdapter2D::doGetTool() {
            return m_tool;
        }
        
        bool CreateBrushToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 planeNorm(camera.direction().firstAxis());
            plane = Plane3(initialPoint, planeNorm);
            
            const Ray3& pickRay = inputState.pickRay();
            initialPoint = pickRay.pointAtDistance(plane.intersectWithRay(pickRay));

            m_initialPoint = initialPoint;
            m_bounds.min = m_bounds.max = m_initialPoint;
            snapBounds(inputState);
            
            return true;
        }
        
        bool CreateBrushToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            
            m_bounds.min = m_bounds.max = m_initialPoint;
            m_bounds.mergeWith(curPoint);
            snapBounds(inputState);
            
            return true;
        }
        
        void CreateBrushToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush* brush = builder.createCuboid(m_bounds, document->currentTextureName());
            
            const Transaction transaction(document, "Create brush");
            document->deselectAll();
            document->addNode(brush, document->currentLayer());
            document->select(brush);
        }
        
        void CreateBrushToolAdapter2D::doCancelPlaneDrag() {}
        
        void CreateBrushToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        void CreateBrushToolAdapter2D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        
        void CreateBrushToolAdapter2D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);
                renderService.renderBounds(BBox3f(m_bounds));
            }
        }

        bool CreateBrushToolAdapter2D::doCancel() {
            return false;
        }

        void CreateBrushToolAdapter2D::snapBounds(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            m_bounds.min = grid.snapDown(m_bounds.min);
            m_bounds.max = grid.snapUp(m_bounds.max);
            
            const Renderer::Camera& camera = inputState.camera();
            const BBox3& refBounds = document->referenceBounds();
            m_bounds = m_bounds.mixed(refBounds, camera.direction().firstAxis().absolute());
        }
    }
}
