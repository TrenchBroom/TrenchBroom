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

#include "RenderService.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/PrimitiveRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/TextRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        Renderer::FontDescriptor makeRenderServiceFont();
        Renderer::FontDescriptor makeRenderServiceFont() {
            return Renderer::FontDescriptor(pref(Preferences::RendererFontPath()), static_cast<size_t>(pref(Preferences::RendererFontSize)));
        }
        
        RenderService::RenderService(RenderContext& renderContext, RenderBatch& renderBatch) :
        m_renderContext(renderContext),
        m_renderBatch(renderBatch),
        m_textRenderer(new TextRenderer(makeRenderServiceFont())),
        m_pointHandleRenderer(new PointHandleRenderer()),
        m_primitiveRenderer(new PrimitiveRenderer()),
        m_foregroundColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_backgroundColor(0.0f, 0.0f, 0.0f, 1.0f),
        m_lineWidth(1.0f) {}
        
        RenderService::~RenderService() {
            flush();
        }

        void RenderService::setForegroundColor(const Color& foregroundColor) {
            m_foregroundColor = foregroundColor;
        }
        
        void RenderService::setBackgroundColor(const Color& backgroundColor) {
            m_backgroundColor = backgroundColor;
        }
        
        void RenderService::setLineWidth(const float lineWidth) {
            m_lineWidth = lineWidth;
        }

        void RenderService::renderString(const AttrString& string, const Vec3f& position) {
            renderString(string, SimpleTextAnchor(position, TextAlignment::Bottom, Vec2f(0.0f, 16.0f)));
        }

        void RenderService::renderString(const AttrString& string, const TextAnchor& position) {
            m_textRenderer->renderString(m_renderContext, m_foregroundColor, m_backgroundColor, string, position);
        }
        
        void RenderService::renderStringOnTop(const AttrString& string, const Vec3f& position) {
            renderStringOnTop(string, SimpleTextAnchor(position, TextAlignment::Bottom, Vec2f(0.0f, 16.0f)));
        }

        void RenderService::renderStringOnTop(const AttrString& string, const TextAnchor& position) {
            m_textRenderer->renderStringOnTop(m_renderContext, m_foregroundColor, m_backgroundColor, string, position);
        }

        void RenderService::renderPointHandles(const Vec3f::List& positions) {
            Vec3f::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                renderPointHandle(*it);
        }
        
        void RenderService::renderPointHandle(const Vec3f& position) {
            m_pointHandleRenderer->addPoint(m_foregroundColor, position);
        }

        void RenderService::renderPointHandleHighlight(const Vec3f& position) {
            m_pointHandleRenderer->addHighlight(m_foregroundColor, position);
        }

        void RenderService::renderLine(const Vec3f& start, const Vec3f& end) {
            m_primitiveRenderer->renderLine(m_foregroundColor, m_lineWidth, start, end);
        }
        
        void RenderService::renderLines(const Vec3f::List& positions) {
            m_primitiveRenderer->renderLines(m_foregroundColor, m_lineWidth, positions);
        }

        void RenderService::renderCoordinateSystem(const BBox3f& bounds) {
            const Color& x = pref(Preferences::XAxisColor);
            const Color& y = pref(Preferences::YAxisColor);
            const Color& z = pref(Preferences::ZAxisColor);
            
            if (m_renderContext.render2D()) {
                const Camera& camera = m_renderContext.camera();
                const Math::Axis::Type axis = camera.direction().firstComponent();
                switch (axis) {
                    case Math::Axis::AX:
                        m_primitiveRenderer->renderCoordinateSystemYZ(y, z, m_lineWidth, bounds);
                        break;
                    case Math::Axis::AY:
                        m_primitiveRenderer->renderCoordinateSystemXZ(x, z, m_lineWidth, bounds);
                        break;
                    default:
                        m_primitiveRenderer->renderCoordinateSystemXY(x, y, m_lineWidth, bounds);
                        break;
                }
            } else {
                m_primitiveRenderer->renderCoordinateSystem3D(x, y, z, m_lineWidth, bounds);
            }
        }
        
        void RenderService::renderPolygonOutline(const Vec3f::List& positions) {
            m_primitiveRenderer->renderPolygon(m_foregroundColor, m_lineWidth, positions);
        }

        void RenderService::renderFilledPolygon(const Vec3f::List& positions) {
            m_primitiveRenderer->renderFilledPolygon(m_foregroundColor, positions);
        }

        void RenderService::renderBounds(const BBox3f& bounds) {
            const Vec3f p1(bounds.min.x(), bounds.min.y(), bounds.min.z());
            const Vec3f p2(bounds.min.x(), bounds.min.y(), bounds.max.z());
            const Vec3f p3(bounds.min.x(), bounds.max.y(), bounds.min.z());
            const Vec3f p4(bounds.min.x(), bounds.max.y(), bounds.max.z());
            const Vec3f p5(bounds.max.x(), bounds.min.y(), bounds.min.z());
            const Vec3f p6(bounds.max.x(), bounds.min.y(), bounds.max.z());
            const Vec3f p7(bounds.max.x(), bounds.max.y(), bounds.min.z());
            const Vec3f p8(bounds.max.x(), bounds.max.y(), bounds.max.z());
            
            Vec3f::List positions;
            positions.reserve(12 * 2);
            positions.push_back(p1); positions.push_back(p2);
            positions.push_back(p1); positions.push_back(p3);
            positions.push_back(p1); positions.push_back(p5);
            positions.push_back(p2); positions.push_back(p4);
            positions.push_back(p2); positions.push_back(p6);
            positions.push_back(p3); positions.push_back(p4);
            positions.push_back(p3); positions.push_back(p7);
            positions.push_back(p4); positions.push_back(p8);
            positions.push_back(p5); positions.push_back(p6);
            positions.push_back(p5); positions.push_back(p7);
            positions.push_back(p6); positions.push_back(p8);
            positions.push_back(p7); positions.push_back(p8);
            
            renderLines(positions);
        }

        void RenderService::renderCircle(const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            m_primitiveRenderer->renderCircle(m_foregroundColor, m_lineWidth, position, normal, segments, radius, startAxis, endAxis);
        }
        
        void RenderService::renderCircle(const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            m_primitiveRenderer->renderCircle(m_foregroundColor, m_lineWidth, position, normal, segments, radius, startAngle, angleLength);
        }
        
        void RenderService::renderFilledCircle(const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            m_primitiveRenderer->renderFilledCircle(m_foregroundColor, position, normal, segments, radius, startAxis, endAxis);
        }
        
        void RenderService::renderFilledCircle(const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            m_primitiveRenderer->renderFilledCircle(m_foregroundColor, position, normal, segments, radius, startAngle, angleLength);
        }
        
        void RenderService::flush() {
            m_renderBatch.addOneShot(m_primitiveRenderer);
            m_renderBatch.addOneShot(m_pointHandleRenderer);
            m_renderBatch.addOneShot(m_textRenderer);
        }
    }
}
