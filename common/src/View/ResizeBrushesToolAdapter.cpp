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

#include "Hit.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Reference.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
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
        
        void ResizeBrushesToolAdapter::doPick(const InputState& inputState, Hits& hits) {
            if (handleInput(inputState)) {
                const Hit hit = m_tool->pick(inputState.pickRay(), inputState.hits());
                if (hit.isMatch())
                    hits.addHit(hit);
            }
        }
        
        void ResizeBrushesToolAdapter::doModifierKeyChange(const InputState& inputState) {
            updateDragFaces(inputState);
        }
        
        void ResizeBrushesToolAdapter::doMouseMove(const InputState& inputState) {
            updateDragFaces(inputState);
        }
        
        bool ResizeBrushesToolAdapter::doStartMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState))
                return false;
            const bool split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            if (m_tool->beginResize(inputState.hits(), split)) {
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
        }
        
        void ResizeBrushesToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (!handleInput(inputState) || !m_tool->hasDragFaces())
                return;
            
            Renderer::EdgeRenderer edgeRenderer = buildEdgeRenderer();
            Renderer::RenderEdges* renderEdges = new Renderer::RenderEdges(Reference::swap(edgeRenderer));
            renderEdges->setRenderOccluded();
            renderEdges->setColor(pref(Preferences::ResizeHandleColor));
            renderBatch.addOneShot(renderEdges);
        }

        Renderer::EdgeRenderer ResizeBrushesToolAdapter::buildEdgeRenderer() {
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            
            const Model::BrushFaceList& dragFaces = m_tool->dragFaces();
            Model::BrushFaceList::const_iterator faceIt, faceEnd;
            Model::BrushEdgeList::const_iterator edgeIt, edgeEnd;
            for (faceIt = dragFaces.begin(), faceEnd = dragFaces.end(); faceIt != faceEnd; ++faceIt) {
                const Model::BrushFace* face = *faceIt;
                const Model::BrushEdgeList& edges = face->edges();
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::BrushEdge* edge = *edgeIt;
                    vertices.push_back(Vertex(edge->start->position));
                    vertices.push_back(Vertex(edge->end->position));
                }
            }
            
            return Renderer::EdgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
        }
        
        bool ResizeBrushesToolAdapter::doCancel() {
            return false;
        }

        void ResizeBrushesToolAdapter::updateDragFaces(const InputState& inputState) {
            if (handleInput(inputState) && !dragging())
                m_tool->updateDragFaces(inputState.hits());
        }

        bool ResizeBrushesToolAdapter::handleInput(const InputState& inputState) const {
            return ((inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                     inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd)) &&
                    m_tool->applies());
        }
    }
}
