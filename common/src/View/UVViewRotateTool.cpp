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

#include "UVViewRotateTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushFace.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVViewRotateTool::AngleHandleHit = Hit::freeHitType();
        const float UVViewRotateTool::HandleRadius = 5.0f;
        const float UVViewRotateTool::HandleLength = 32.0f;

        UVViewRotateTool::UVViewRotateTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        void UVViewRotateTool::doPick(const InputState& inputState, Hits& hits) {
            if (!m_helper.valid())
                return;

            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            
            const Ray3& pickRay = inputState.pickRay();
            const FloatType distance = pickRay.intersectWithPlane(boundary.normal, boundary.anchor());
            assert(!Math::isnan(distance));
            const Vec3 hitPoint = pickRay.pointAtDistance(distance);
            
            Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);
            const Vec2f hitPointInFaceCoords = Vec2f(faceCoordSystem.worldToTex(hitPoint));
            
            const Vec2f angleHandleInFaceCoords = angleHandle();
            const float angleHandleError = hitPointInFaceCoords.distanceTo(angleHandleInFaceCoords);
            if (Math::abs(angleHandleError) <= 2.0f * HandleRadius / m_helper.cameraZoom())
                hits.addHit(Hit(AngleHandleHit, distance, hitPoint, 0, angleHandleError));
        }
        
        bool UVViewRotateTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;

            const Hits& hits = inputState.hits();
            const Hit& angleHandleHit = hits.findFirst(AngleHandleHit, true);

            if (!angleHandleHit.isMatch())
                return false;

            const Model::BrushFace* face = m_helper.face();
            Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);

            const Vec2f hitPointInFaceCoords = faceCoordSystem.worldToTex(angleHandleHit.hitPoint());
            const Vec2f angleHandleInFaceCoords = angleHandle();
            m_offset = hitPointInFaceCoords - angleHandleInFaceCoords;
            controller()->beginUndoableGroup("Rotate Texture");
            
            return true;
        }
        
        bool UVViewRotateTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const Ray3& pickRay = inputState.pickRay();
            const FloatType curPointDistance = pickRay.intersectWithPlane(boundary.normal, boundary.anchor());
            const Vec3 curPoint = pickRay.pointAtDistance(curPointDistance);
            
            Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);
            const Vec2f curPointInFaceCoords = Vec2f(faceCoordSystem.worldToTex(curPoint));
            
            const Vec2f oldCenterInFaceCoords = m_helper.originInFaceCoords();
            const Vec3 oldCenterInWorldCoords = faceCoordSystem.texToWorld(Vec3(oldCenterInFaceCoords));
            const float angle = measureAngle(curPointInFaceCoords);
            const float snappedAngle = snapAngle(angle);
            
            const Model::BrushFaceList applyTo(1, face);
            controller()->setFaceRotation(applyTo, snappedAngle, false);
            
            // Correct the offsets and the position of the rotation center.
            const Vec2f newCenterInFaceCoords(faceCoordSystem.worldToTex(oldCenterInWorldCoords));
            const Vec2f delta = (oldCenterInFaceCoords - newCenterInFaceCoords) / face->scale();
            controller()->setFaceOffset(applyTo, delta, true);
            m_helper.setOrigin(newCenterInFaceCoords);
            
            return true;
        }
        
        float UVViewRotateTool::measureAngle(const Vec2f& point) const {
            const Model::BrushFace* face = m_helper.face();
            const Vec2f origin = m_helper.originInFaceCoords();
            return Math::mod(face->measureTextureAngle(origin, point), 360.0f);
        }
        
        float UVViewRotateTool::snapAngle(const float angle) const {
            const Model::BrushFace* face = m_helper.face();
            
            const float angles[] = {
                Math::mod(angle +   0.0f, 360.0f),
                Math::mod(angle +  90.0f, 360.0f),
                Math::mod(angle + 180.0f, 360.0f),
                Math::mod(angle + 270.0f, 360.0f),
            };
            float minDelta = std::numeric_limits<float>::max();
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);
            const Model::BrushEdgeList& edges = face->edges();
            Model::BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::BrushEdge* edge = *it;
                
                const Vec3 startInFaceCoords = faceCoordSystem.worldToTex(edge->start->position);
                const Vec3 endInFaceCoords   = faceCoordSystem.worldToTex(edge->end->position);
                const float edgeAngle        = Math::mod(face->measureTextureAngle(startInFaceCoords, endInFaceCoords), 360.0f);
                
                for (size_t i = 0; i < 4; ++i) {
                    if (std::abs(angles[i] - edgeAngle) < std::abs(minDelta))
                        minDelta = angles[i] - edgeAngle;
                }
            }
            
            if (std::abs(minDelta) < 3.0f)
                return angle - minDelta;
            return angle;
        }

        void UVViewRotateTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void UVViewRotateTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }

        void UVViewRotateTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            const Hits& hits = inputState.hits();
            const Hit& angleHandleHit = hits.findFirst(AngleHandleHit, true);
            
            const bool highlightAngleHandle = angleHandleHit.isMatch() || dragging();
            
            const PreferenceManager& prefs = PreferenceManager::instance();
            const Color& handleColor = prefs.get(Preferences::HandleColor);
            const Color& highlightColor = prefs.get(Preferences::SelectedHandleColor);
            const float cameraZoom = m_helper.cameraZoom();

            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_helper.face());
            const Vec2f originPosition = m_helper.originInFaceCoords();
            const Vec2f angleHandlePosition = angleHandle();
            const Vec2f faceCenterPosition = faceCoordSystem.worldToTex(m_helper.face()->center());

            const float actualRadius = HandleRadius / cameraZoom;
            
            Renderer::Vbo vbo(0xFFF);
            Renderer::SetVboState vboState(vbo);
            Renderer::Circle center(actualRadius / 2.0f, 10, true);
            Renderer::Circle fill(actualRadius, 16, true);
            Renderer::Circle highlight(actualRadius * 2.0f, 16, false);
            Renderer::Circle outer(HandleLength / cameraZoom, 64, false);

            typedef Renderer::VertexSpecs::P2::Vertex Vertex;
            Vertex::List lineVertices(2);
            lineVertices[0] = Vertex(originPosition);
            lineVertices[1] = Vertex(angleHandlePosition);
            Renderer::VertexArray array = Renderer::VertexArray::ref(GL_LINES, lineVertices);
            
            vboState.mapped();
            center.prepare(vbo);
            fill.prepare(vbo);
            highlight.prepare(vbo);
            outer.prepare(vbo);
            array.prepare(vbo);
            vboState.active();

            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            const Renderer::MultiplyModelMatrix toWorldTransform(renderContext.transformation(), faceCoordSystem.toWorldMatrix());
            {
                const Mat4x4 translation = translationMatrix(Vec3(originPosition));
                const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), translation);
                shader.set("Color", handleColor);
                fill.render();
                outer.render();
            }
            
            {
                const Mat4x4 translation = translationMatrix(Vec3(angleHandlePosition));
                const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), translation);
                shader.set("Color", handleColor);
                fill.render();
                
                if (highlightAngleHandle) {
                    shader.set("Color", highlightColor);
                    highlight.render();
                }
            }
            
            shader.set("Color", handleColor);
            array.render();

            {
                const Mat4x4 translation = translationMatrix(Vec3(faceCenterPosition));
                const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), translation);
                shader.set("Color", highlightColor);
                center.render();
            }
        }

        Vec2f UVViewRotateTool::angleHandle() const {
            const float distance = HandleLength / m_helper.cameraZoom();
            return m_helper.originInFaceCoords() + distance * Vec2f::PosX;
        }
    }
}
