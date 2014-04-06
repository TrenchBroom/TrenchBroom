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

#include "TexturingViewRotateTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushFace.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/Circle.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewRotateTool::CenterHandleHit = Hit::freeHitType();
        const Hit::HitType TexturingViewRotateTool::AngleHandleHit = Hit::freeHitType();
        const FloatType TexturingViewRotateTool::HandleRadius = 5.0;
        const float TexturingViewRotateTool::HandleLength = 32.0f;

        TexturingViewRotateTool::TexturingViewRotateTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera),
        m_dragMode(None) {}
        
        void TexturingViewRotateTool::doPick(const InputState& inputState, Hits& hits) {
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
            
            const Vec2f centerHandleInFaceCoords = m_helper.rotationCenterInFaceCoords();
            const float centerHandleError = hitPointInFaceCoords.distanceTo(centerHandleInFaceCoords);
            if (Math::abs(centerHandleError) <= 2.0f * HandleRadius / m_camera.zoom().x())
                hits.addHit(Hit(CenterHandleHit, distance, hitPoint, 0, centerHandleError));
            
            const Vec2f angleHandleInFaceCoords = m_helper.angleHandleInFaceCoords(HandleLength / m_camera.zoom().x());
            const float angleHandleError = hitPointInFaceCoords.distanceTo(angleHandleInFaceCoords);
            if (Math::abs(angleHandleError) <= 2.0f * HandleRadius / m_camera.zoom().x())
                hits.addHit(Hit(AngleHandleHit, distance, hitPoint, 0, angleHandleError));
        }
        
        bool TexturingViewRotateTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;

            const Hits& hits = inputState.hits();
            const Hit& centerHandleHit = hits.findFirst(CenterHandleHit, true);
            const Hit& angleHandleHit = hits.findFirst(AngleHandleHit, true);

            if (!centerHandleHit.isMatch() && !angleHandleHit.isMatch())
                return false;

            const Model::BrushFace* face = m_helper.face();
            Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);

            if (centerHandleHit.isMatch()) {
                const Vec2f hitPointInFaceCoords = faceCoordSystem.worldToTex(centerHandleHit.hitPoint());
                const Vec2f centerHandleInFaceCoords = m_helper.rotationCenterInFaceCoords();
                m_offset = hitPointInFaceCoords - centerHandleInFaceCoords;
                m_dragMode = Center;
            } else if (angleHandleHit.isMatch()) {
                const Vec2f hitPointInFaceCoords = faceCoordSystem.worldToTex(centerHandleHit.hitPoint());
                const Vec2f angleHandleInFaceCoords = m_helper.angleHandleInFaceCoords(HandleLength / m_camera.zoom().x());
                m_offset = hitPointInFaceCoords - angleHandleInFaceCoords;
                m_dragMode = Angle;
            }
            
            return true;
        }
        
        bool TexturingViewRotateTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            assert(m_dragMode != None);
            
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const Ray3& pickRay = inputState.pickRay();
            const FloatType curPointDistance = pickRay.intersectWithPlane(boundary.normal, boundary.anchor());
            const Vec3 curPoint = pickRay.pointAtDistance(curPointDistance);
            
            Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(face);
            const Vec2f curPointInFaceCoords = Vec2f(faceCoordSystem.worldToTex(curPoint));
            
            if (m_dragMode == Center) {
                Vec3::List snapPoints = Model::vertexPositions(face->vertices());
                snapPoints.push_back(face->center());
                const Vec2f snappedPoint = m_helper.snapToPoints(curPointInFaceCoords - m_offset, snapPoints);
                m_helper.setRotationCenter(snappedPoint);
            } else {
            }
            
            return true;
        }
        
        void TexturingViewRotateTool::doEndMouseDrag(const InputState& inputState) {
            m_dragMode = None;
        }
        
        void TexturingViewRotateTool::doCancelMouseDrag(const InputState& inputState) {
            m_dragMode = None;
        }

        void TexturingViewRotateTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (!m_helper.valid())
                return;
            
            const Hits& hits = inputState.hits();
            const Hit& centerHandleHit = hits.findFirst(CenterHandleHit, true);
            const Hit& angleHandleHit = hits.findFirst(AngleHandleHit, true);
            
            const bool highlightCenterHandle = centerHandleHit.isMatch() || m_dragMode == Center;
            const bool highlightAngleHandle = angleHandleHit.isMatch() || m_dragMode == Angle;
            
            const PreferenceManager& prefs = PreferenceManager::instance();
            const Color& handleColor = prefs.get(Preferences::HandleColor);
            const Color& highlightColor = prefs.get(Preferences::SelectedHandleColor);

            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_helper.face());
            const Vec2f centerHandlePosition = m_helper.rotationCenterInFaceCoords();
            const Vec2f angleHandlePosition = m_helper.angleHandleInFaceCoords(HandleLength / m_camera.zoom().x());
            const Vec2f faceCenterPosition = faceCoordSystem.worldToTex(m_helper.face()->center());

            const float actualRadius = HandleRadius / m_camera.zoom().x();
            
            Renderer::Vbo vbo(0xFFF);
            Renderer::SetVboState vboState(vbo);
            Renderer::Circle center(actualRadius / 2.0f, 10, true);
            Renderer::Circle fill(actualRadius, 16, true);
            Renderer::Circle highlight(actualRadius * 2.0f, 16, false);
            Renderer::Circle outer(HandleLength / m_camera.zoom().x(), 64, false);

            typedef Renderer::VertexSpecs::P2::Vertex Vertex;
            Vertex::List lineVertices(2);
            lineVertices[0] = Vertex(centerHandlePosition);
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
                const Mat4x4 translation = translationMatrix(Vec3(centerHandlePosition));
                const Renderer::MultiplyModelMatrix centerTransform(renderContext.transformation(), translation);
                shader.set("Color", handleColor);
                fill.render();
                outer.render();
                
                if (highlightCenterHandle) {
                    shader.set("Color", highlightColor);
                    highlight.render();
                }
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
    }
}
