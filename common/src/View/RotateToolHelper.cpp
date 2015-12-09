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

#include "RotateToolHelper.h"

#include "AttrString.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/GL.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        RotateToolDelegate::~RotateToolDelegate() {}
        
        bool RotateToolDelegate::handleRotate(const InputState& inputState) const {
            return doHandleRotate(inputState);
        }
        
        RotateInfo RotateToolDelegate::getRotateInfo(const InputState& inputState) const {
            return doGetRotateInfo(inputState);
        }
        
        bool RotateToolDelegate::startRotate(const InputState& inputState) {
            return doStartRotate(inputState);
        }
        
        FloatType RotateToolDelegate::getAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const {
            return doGetAngle(inputState, handlePoint, curPoint, axis);
        }
        
        bool RotateToolDelegate::rotate(const Vec3& center, const Vec3& axis, const FloatType angle) {
            return doRotate(center, axis, angle);
        }
        
        void RotateToolDelegate::endRotate(const InputState& inputState) {
            doEndRotate(inputState);
        }
        
        void RotateToolDelegate::cancelRotate() {
            doCancelRotate();
        }
        
        const size_t RotateToolHelper::SnapAngleKey = 1;
        const size_t RotateToolHelper::AngleKey = 2;

        RotateToolHelper::RotateToolHelper(PlaneDragPolicy* policy, RotateToolDelegate& delegate) :
        PlaneDragHelper(policy),
        m_delegate(delegate),
        m_lastAngle(0.0) {}
        
        bool RotateToolHelper::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            if (!m_delegate.handleRotate(inputState))
                return false;
            
            const RotateInfo info = m_delegate.getRotateInfo(inputState);
            initialPoint = info.origin;
            plane = info.plane;
            m_center = info.center;
            m_axis = info.axis;
            m_firstPoint = initialPoint;
            m_lastAngle = 0.0;
            
            if (!m_delegate.startRotate(inputState))
                return false;
            return true;
        }
        
        bool RotateToolHelper::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            const FloatType angle = m_delegate.getAngle(inputState, refPoint, curPoint, m_axis);
            if (angle == m_lastAngle)
                return true;
            if (!m_delegate.rotate(m_center, m_axis, angle))
                return false;
            m_lastAngle = angle;

            return true;
        }
        
        void RotateToolHelper::doEndPlaneDrag(const InputState& inputState) {
            m_delegate.endRotate(inputState);
            m_lastAngle = 0.0;
        }
        
        void RotateToolHelper::doCancelPlaneDrag() {
            m_delegate.cancelRotate();
            m_lastAngle = 0.0;
        }
        
        void RotateToolHelper::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        void RotateToolHelper::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (!dragging())
                return;
            
            renderAngleIndicator(renderContext, renderBatch);
            renderText(renderContext, renderBatch);
        }

        class RotateToolHelper::AngleIndicatorRenderer : public Renderer::DirectRenderable {
        private:
            Vec3 m_position;
            Renderer::Circle m_circle;
        public:
            AngleIndicatorRenderer(const Vec3& position, const float radius, const Math::Axis::Type axis, const Vec3& startAxis, const Vec3& endAxis) :
            m_position(position),
            m_circle(radius, 24, true, axis, startAxis, endAxis) {}
        private:
            void doPrepareVertices(Renderer::Vbo& vertexVbo) {
                m_circle.prepare(vertexVbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) {
                glAssert(glDisable(GL_DEPTH_TEST));
                glAssert(glDisable(GL_CULL_FACE));
                glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
                
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", Color(1.0f, 1.0f, 1.0f, 0.2f));
                m_circle.render();

                glAssert(glPolygonMode(GL_FRONT, GL_FILL));
                glAssert(glEnable(GL_CULL_FACE));
                glAssert(glEnable(GL_DEPTH_TEST));
            }
        };
        
        void RotateToolHelper::renderAngleIndicator(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Vec3 startAxis = (m_firstPoint - m_center).normalized();
            const Vec3 endAxis = Quat3(m_axis, m_lastAngle) * startAxis;

            renderBatch.addOneShot(new AngleIndicatorRenderer(m_center, handleRadius, m_axis.firstComponent(), startAxis, endAxis));
        }
        
        void RotateToolHelper::renderText(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            Renderer::RenderService renderService(renderContext, renderBatch);
            
            renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderService.renderStringOnTop(angleString(Math::degrees(m_lastAngle)), m_center);
        }

        String RotateToolHelper::angleString(const FloatType angle) const {
            StringStream str;
            str.precision(2);
            str.setf(std::ios::fixed);
            str << angle;
            return str.str();
        }
    }
}
