/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "ScaleObjectsToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Reference.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/ScaleObjectsTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ScaleObjectsToolController::ScaleObjectsToolController(ScaleObjectsTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
        }
        
        ScaleObjectsToolController::~ScaleObjectsToolController() {}
        
        Tool* ScaleObjectsToolController::doGetTool() {
            return m_tool;
        }
        
        void ScaleObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (handleInput(inputState)) {
                const Model::Hit hit = doPick(inputState.pickRay(), inputState.camera(), pickResult);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
        }
        
        void ScaleObjectsToolController::doModifierKeyChange(const InputState& inputState) {
//            if (!anyToolDragging(inputState))
//                m_tool->updateDragFaces(inputState.pickResult());
            if (thisToolDragging()) {
                updateResize(inputState);
            }
        }
        
        void ScaleObjectsToolController::doMouseMove(const InputState& inputState) {
            if (handleInput(inputState) && !anyToolDragging(inputState))
                m_tool->updateDragFaces(inputState.pickResult());
        }
        
        bool ScaleObjectsToolController::doStartMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState))
                return false;
            
            m_tool->updateDragFaces(inputState.pickResult());
            const bool split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            if (m_tool->beginResize(inputState.pickResult(), split)) {
                m_tool->updateDragFaces(inputState.pickResult());
                return true;
            }
            return false;
        }
        
        bool ScaleObjectsToolController::doMouseDrag(const InputState& inputState) {
            return updateResize(inputState);
        }
        
        bool ScaleObjectsToolController::updateResize(const InputState& inputState) {
            return m_tool->resize(inputState.pickRay(),
                                  inputState.camera(),
                                  inputState.modifierKeysDown(ModifierKeys::MKShift),
                                  inputState.modifierKeysDown(ModifierKeys::MKAlt),
                                  inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
        }
        
        void ScaleObjectsToolController::doEndMouseDrag(const InputState& inputState) {
            m_tool->commitResize();
            m_tool->updateDragFaces(inputState.pickResult());
        }
        
        void ScaleObjectsToolController::doCancelMouseDrag() {
            m_tool->cancelResize();
        }
        
        void ScaleObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (thisToolDragging())
                renderContext.setForceShowSelectionGuide();
            // TODO: force rendering of all other map views if the input applies and the tool has drag faces
        }
        
        void ScaleObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            
            // regular indicators
            
            
            renderService.setForegroundColor(Color(0, 255, 0));
            renderService.renderBounds(m_tool->bounds());
            
            for (const Vec3& corner : m_tool->cornerHandles()) {
                renderService.renderHandle(corner);
            }
            
            
            // highlighted stuff
            
            if (m_tool->hasDragPolygon()) {
                Renderer::DirectEdgeRenderer edgeRenderer = buildEdgeRendererForSide();
                edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
            }
            
            if (m_tool->hasDragEdge()) {
                Renderer::DirectEdgeRenderer edgeRenderer = buildEdgeRendererForEdge();
                edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
            }
            
            if (m_tool->hasDragCorner()) {
                renderService.renderHandleHighlight(m_tool->dragCorner());
            }
        }
        
        Renderer::DirectEdgeRenderer ScaleObjectsToolController::buildEdgeRendererForEdge() {
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            
            const Edge3 edge = m_tool->dragEdge();
            vertices.push_back(Vertex(edge.start()));
            vertices.push_back(Vertex(edge.end()));
        
            return Renderer::DirectEdgeRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
        }
        
        Renderer::DirectEdgeRenderer ScaleObjectsToolController::buildEdgeRendererForSide() {
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            
            const Polygon3 poly = m_tool->dragPolygon();
            for (size_t i = 0; i < poly.vertexCount(); ++i) {
                vertices.push_back(Vertex(poly.vertices().at(i)));
                vertices.push_back(Vertex(poly.vertices().at((i + 1) % poly.vertexCount())));
            }
            
            return Renderer::DirectEdgeRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
        }
        
        bool ScaleObjectsToolController::doCancel() {
            return false;
        }
        
        bool ScaleObjectsToolController::handleInput(const InputState& inputState) const {
            return m_tool->applies();
//            return ((inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
//                     inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd)) &&
//                    m_tool->applies());
        }
        
        ScaleObjectsToolController2D::ScaleObjectsToolController2D(ScaleObjectsTool* tool) :
        ScaleObjectsToolController(tool) {}
        
        Model::Hit ScaleObjectsToolController2D::doPick(const Ray3& pickRay, const Renderer::Camera& camera,const Model::PickResult& pickResult) {
            return m_tool->pick2D(pickRay, camera, pickResult);
        }
        
        ScaleObjectsToolController3D::ScaleObjectsToolController3D(ScaleObjectsTool* tool) :
        ScaleObjectsToolController(tool) {}
        
        Model::Hit ScaleObjectsToolController3D::doPick(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) {
            return m_tool->pick3D(pickRay, camera, pickResult);
        }
    }
}

