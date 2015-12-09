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

#include "ResizeBrushesToolAdapter.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Reference.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/ResizeBrushesTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ResizeBrushesToolAdapter::ResizeBrushesToolAdapter(ResizeBrushesTool* tool) :
        m_tool(tool) {
            assert(m_tool != NULL);
        }

        ResizeBrushesToolAdapter::~ResizeBrushesToolAdapter() {}

        Tool* ResizeBrushesToolAdapter::doGetTool() {
            return m_tool;
        }
        
        void ResizeBrushesToolAdapter::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (handleInput(inputState)) {
                const Model::Hit hit = doPick(inputState.pickRay(), pickResult);
                if (hit.isMatch())
                    pickResult.addHit(hit);
            }
        }
        
        void ResizeBrushesToolAdapter::doModifierKeyChange(const InputState& inputState) {
            updateDragFaces(inputState);
        }
        
        void ResizeBrushesToolAdapter::doMouseMove(const InputState& inputState) {
            if (handleInput(inputState))
                updateDragFaces(inputState);
        }
        
        bool ResizeBrushesToolAdapter::doStartMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState))
                return false;
            const bool split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            if (m_tool->beginResize(inputState.pickResult(), split)) {
                updateDragFaces(inputState);
                return true;
            }
            return false;
        }
        
        bool ResizeBrushesToolAdapter::doMouseDrag(const InputState& inputState) {
            return m_tool->resize(inputState.pickRay(), inputState.camera());
        }
        
        void ResizeBrushesToolAdapter::doEndMouseDrag(const InputState& inputState) {
            m_tool->commitResize();
        }
        
        void ResizeBrushesToolAdapter::doCancelMouseDrag() {
            m_tool->cancelResize();
        }
        
        void ResizeBrushesToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (dragging())
                renderContext.setForceShowSelectionGuide();
            // TODO: force rendering of all other map views if the input applies and the tool has drag faces
        }
        
        void ResizeBrushesToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_tool->hasDragFaces()) {
                Renderer::DirectEdgeRenderer edgeRenderer = buildEdgeRenderer();
                edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
            }
        }

        Renderer::DirectEdgeRenderer ResizeBrushesToolAdapter::buildEdgeRenderer() {
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            
            const Model::BrushFaceList& dragFaces = m_tool->dragFaces();
            Model::BrushFaceList::const_iterator faceIt, faceEnd;
            Model::BrushFace::EdgeList::const_iterator edgeIt, edgeEnd;
            for (faceIt = dragFaces.begin(), faceEnd = dragFaces.end(); faceIt != faceEnd; ++faceIt) {
                const Model::BrushFace* face = *faceIt;
                const Model::BrushFace::EdgeList edges = face->edges();
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::BrushEdge* edge = *edgeIt;
                    vertices.push_back(Vertex(edge->firstVertex()->position()));
                    vertices.push_back(Vertex(edge->secondVertex()->position()));
                }
            }
            
            return Renderer::DirectEdgeRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
        }
        
        bool ResizeBrushesToolAdapter::doCancel() {
            return false;
        }

        void ResizeBrushesToolAdapter::updateDragFaces(const InputState& inputState) {
            if (!dragging())
                m_tool->updateDragFaces(inputState.pickResult());
        }

        bool ResizeBrushesToolAdapter::handleInput(const InputState& inputState) const {
            return ((inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                     inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd)) &&
                    m_tool->applies());
        }

        ResizeBrushesToolAdapter2D::ResizeBrushesToolAdapter2D(ResizeBrushesTool* tool) :
        ResizeBrushesToolAdapter(tool) {}

        Model::Hit ResizeBrushesToolAdapter2D::doPick(const Ray3& pickRay, const Model::PickResult& pickResult) {
            return m_tool->pick2D(pickRay, pickResult);
        }
        
        ResizeBrushesToolAdapter3D::ResizeBrushesToolAdapter3D(ResizeBrushesTool* tool) :
        ResizeBrushesToolAdapter(tool) {}
        
        Model::Hit ResizeBrushesToolAdapter3D::doPick(const Ray3& pickRay, const Model::PickResult& pickResult) {
            return m_tool->pick3D(pickRay, pickResult);
        }
    }
}
