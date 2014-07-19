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

#include "UVScaleTool.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/ModelTypes.h"
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexSpec.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"
#include "View/UVOriginTool.h"

namespace TrenchBroom {
    namespace View {
        UVScaleTool::UVScaleTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        UVGridTool(document, controller, helper) {}

        bool UVScaleTool::checkIfDragApplies(const InputState& inputState, const Hit& xHit, const Hit& yHit) const {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            if (!xHit.isMatch() && !yHit.isMatch())
                return false;
            
            return true;
        }
        
        String UVScaleTool::getActionName() const {
            return "Scale Texture";
        }
        
        Vec2f UVScaleTool::performDrag(const Vec2f& dragDeltaFaceCoords) {
            const Vec2f curHandlePosTexCoords  = getScaledTranslatedHandlePos();
            const Vec2f newHandlePosFaceCoords = getHandlePos() + dragDeltaFaceCoords;
            const Vec2f newHandlePosSnapped    = snap(newHandlePosFaceCoords);
            
            const Vec2f originHandlePosFaceCoords = m_helper.originInFaceCoords();
            const Vec2f originHandlePosTexCoords  = m_helper.originInTexCoords();
            
            const Vec2f newHandleDistFaceCoords = newHandlePosSnapped    - originHandlePosFaceCoords;
            const Vec2f curHandleDistTexCoords  = curHandlePosTexCoords  - originHandlePosTexCoords;
            
            Model::BrushFace* face = m_helper.face();
            Vec2f newScale = face->scale();
            for (size_t i = 0; i < 2; ++i)
                if (m_selector[i])
                    newScale[i] = newHandleDistFaceCoords[i] / curHandleDistTexCoords[i];
            newScale.correct(4, 0.0f);
            
            const Model::BrushFaceList applyTo(1, face);
            controller()->setFaceXScale(applyTo, newScale.x(), false);
            controller()->setFaceYScale(applyTo, newScale.y(), false);
            
            const Vec2f newOriginInTexCoords = m_helper.originInTexCoords().corrected(4, 0.0f);
            const Vec2f originDelta = originHandlePosTexCoords - newOriginInTexCoords;
            
            controller()->setFaceXOffset(applyTo, originDelta.x(), true);
            controller()->setFaceYOffset(applyTo, originDelta.y(), true);
            
            return dragDeltaFaceCoords - (newHandlePosFaceCoords - newHandlePosSnapped);
        }
        
        Vec2f UVScaleTool::snap(const Vec2f& position) const {
            const Model::BrushFace* face = m_helper.face();
            const Mat4x4 toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            Vec2f distance = Vec2f::Max;
            
            const Model::BrushVertexList& vertices = face->vertices();
            for (size_t i = 0; i < vertices.size(); ++i) {
                const Vec2f vertex(toTex * vertices[i]->position);
                distance = absMin(distance, position - vertex);
            }
            
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) > 4.0f / m_helper.cameraZoom())
                    distance[i] = 0.0f;
            }
            return position - distance;
        }
        
        void UVScaleTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            // don't overdraw the origin handles
            const Hits& hits = inputState.hits();
            if (hits.findFirst(UVOriginTool::XHandleHit, true).isMatch() ||
                hits.findFirst(UVOriginTool::YHandleHit, true).isMatch())
                return;
                
            EdgeVertex::List vertices = getHandleVertices(hits);
            
            glLineWidth(2.0f);
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        UVScaleTool::EdgeVertex::List UVScaleTool::getHandleVertices(const Hits& hits) const {
            const Hit& xHandleHit = hits.findFirst(XHandleHit, true);
            const Hit& yHandleHit = hits.findFirst(YHandleHit, true);
            const Vec2 stripeSize = m_helper.stripeSize();
            const Color color = Color(1.0f, 1.0f, 0.0f, 1.0f);

            const int xIndex = xHandleHit.target<int>();
            const int yIndex = yHandleHit.target<int>();
            const Vec2 pos = stripeSize * Vec2(xIndex, yIndex);

            Vec3 h1, h2, v1, v2;
            m_helper.computeScaleHandleVertices(pos, v1, v2, h1, h2);

            EdgeVertex::List vertices;
            vertices.resize(4);
            
            if (xHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(Vec3f(v1), color));
                vertices.push_back(EdgeVertex(Vec3f(v2), color));
            }
            
            if (yHandleHit.isMatch()) {
                vertices.push_back(EdgeVertex(Vec3f(h1), color));
                vertices.push_back(EdgeVertex(Vec3f(h2), color));
            }
            
            return vertices;
        }
    }
}
