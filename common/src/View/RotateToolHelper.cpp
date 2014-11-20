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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/GL.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        RotateDelegate::~RotateDelegate() {}
        
        bool RotateDelegate::handleRotate(const InputState& inputState) const {
            return doHandleRotate(inputState);
        }
        
        RotateInfo RotateDelegate::getRotateInfo(const InputState& inputState) const {
            return doGetRotateInfo(inputState);
        }
        
        bool RotateDelegate::startRotate(const InputState& inputState) {
            return doStartRotate(inputState);
        }
        
        FloatType RotateDelegate::getAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const {
            return doGetAngle(inputState, handlePoint, curPoint, axis);
        }
        
        bool RotateDelegate::rotate(const Vec3& center, const Vec3& axis, const FloatType angle) {
            return doRotate(center, axis, angle);
        }
        
        void RotateDelegate::endRotate(const InputState& inputState) {
            doEndRotate(inputState);
        }
        
        void RotateDelegate::cancelRotate() {
            doCancelRotate();
        }
        
        const size_t RotateHelper::SnapAngleKey = 1;
        const size_t RotateHelper::AngleKey = 2;

        RotateHelper::RotateHelper(RotateDelegate& delegate, const Renderer::FontDescriptor& fontDescriptor) :
        m_delegate(delegate),
        m_lastAngle(0.0),
        m_textRenderer(fontDescriptor) {
            m_textRenderer.setFadeDistance(10000.0f);
        }
        
        bool RotateHelper::startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
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
            
            m_textRenderer.addString(AngleKey, angleString(Math::degrees(m_lastAngle)), angleAnchor());
            
            if (!m_delegate.startRotate(inputState))
                return false;
            return true;
        }
        
        bool RotateHelper::planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            const FloatType angle = m_delegate.getAngle(inputState, refPoint, curPoint, m_axis);
            if (angle == m_lastAngle)
                return true;
            if (!m_delegate.rotate(m_center, m_axis, angle))
                return false;
            m_lastAngle = angle;

            m_textRenderer.updateString(AngleKey, angleString(Math::degrees(m_lastAngle)));
            return true;
        }
        
        void RotateHelper::endPlaneDrag(const InputState& inputState) {
            m_delegate.endRotate(inputState);
            m_lastAngle = 0.0;
            m_textRenderer.removeString(AngleKey);
        }
        
        void RotateHelper::cancelPlaneDrag() {
            m_delegate.cancelRotate();
            m_lastAngle = 0.0;
            m_textRenderer.removeString(AngleKey);
        }
        
        void RotateHelper::resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint){}
        
        void RotateHelper::render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (!dragging)
                return;
            
            renderAngleIndicator(renderContext, renderBatch);
            renderText(renderContext, renderBatch);
        }

        class RotateHelper::AngleIndicatorRenderer : public Renderer::Renderable {
        private:
            Vec3 m_position;
            Renderer::Circle m_circle;
        public:
            AngleIndicatorRenderer(const Vec3& position, const float radius, const Math::Axis::Type axis, const Vec3& startAxis, const Vec3& endAxis) :
            m_position(position),
            m_circle(radius, 24, true, axis, startAxis, endAxis) {}
        private:
            void doPrepare(Renderer::Vbo& vbo) {
                m_circle.prepare(vbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) {
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                
                Renderer::MultiplyModelMatrix translation(renderContext.transformation(), translationMatrix(m_position));
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                shader.set("Color", Color(1.0f, 1.0f, 1.0f, 0.2f));
                m_circle.render();

                glPolygonMode(GL_FRONT, GL_FILL);
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
            }
        };
        
        void RotateHelper::renderAngleIndicator(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float handleRadius = static_cast<float>(prefs.get(Preferences::RotateHandleRadius));
            const Vec3 startAxis = (m_firstPoint - m_center).normalized();
            const Vec3 endAxis = Quat3(m_axis, m_lastAngle) * startAxis;

            renderBatch.addOneShot(new AngleIndicatorRenderer(m_center, handleRadius, m_axis.firstComponent(), startAxis, endAxis));
        }
        
        class TextRendererHelper : public RotateHelper::TextRenderer::TextColorProvider, public RotateHelper::TextRenderer::TextRendererFilter {
        public:
            Color textColor(Renderer::RenderContext& context, const size_t& key) const {
                PreferenceManager& prefs = PreferenceManager::instance();
                return prefs.get(Preferences::SelectedInfoOverlayTextColor);
            }
            
            Color backgroundColor(Renderer::RenderContext& context, const size_t& key) const {
                PreferenceManager& prefs = PreferenceManager::instance();
                return prefs.get(Preferences::SelectedInfoOverlayBackgroundColor);
            }

            bool stringVisible(Renderer::RenderContext& context, const size_t& key) const {
                return true;
            }
        };
        
        void RotateHelper::renderText(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            TextRendererHelper helper;
            m_textRenderer.renderOnTop(renderContext, renderBatch, helper, helper,
                                       Renderer::Shaders::TextShader,
                                       Renderer::Shaders::TextBackgroundShader);
        }

        String RotateHelper::angleString(const FloatType angle) const {
            StringStream str;
            str.precision(2);
            str.setf(std::ios::fixed);
            str << angle;
            return str.str();
        }

        Renderer::TextAnchor::Ptr RotateHelper::angleAnchor() const {
            return Renderer::TextAnchor::Ptr(new Renderer::SimpleTextAnchor(m_center, Renderer::Alignment::Bottom | Renderer::Alignment::Center, Vec2f(0.0f, 10.0f)));
        }
    }
}
