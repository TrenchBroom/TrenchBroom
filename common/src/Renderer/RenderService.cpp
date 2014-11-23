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
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/PrimitiveRenderer.h"
#include "Renderer/RenderBatch.h"

namespace TrenchBroom {
    namespace Renderer {
        RenderService::RenderService() :
        m_pointHandleRenderer(new PointHandleRenderer()),
        m_primitiveRenderer(new PrimitiveRenderer()) {}
        
        RenderService::~RenderService() {
            delete m_pointHandleRenderer;
            delete m_primitiveRenderer;
        }

        void RenderService::renderPointHandles(const Vec3f::List& positions) {
            Vec3f::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                renderPointHandle(*it);
        }
        
        void RenderService::renderPointHandle(const Vec3f& position) {
            m_pointHandleRenderer->addPoint(position);
        }
        
        void RenderService::renderSelectedPointHandles(const Vec3f::List& positions) {
            Vec3f::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                renderSelectedPointHandle(*it);
        }

        void RenderService::renderSelectedPointHandle(const Vec3f& position) {
            m_pointHandleRenderer->addSelectedPoint(position);
        }

        void RenderService::renderPointHandleHighlight(const Vec3f& position) {
            m_pointHandleRenderer->addHighlight(position);
        }

        void RenderService::renderLine(const Color& color, const Vec3f& start, const Vec3f& end) {
            m_primitiveRenderer->renderLine(color, start, end);
        }
        
        void RenderService::renderLines(const Color& color, const Vec3f::List& positions) {
            m_primitiveRenderer->renderLines(color, positions);
        }

        void RenderService::renderCoordinateSystem(const BBox3f& bounds) {
            m_primitiveRenderer->renderCoordinateSystem(bounds, pref(Preferences::XAxisColor), pref(Preferences::YAxisColor), pref(Preferences::ZAxisColor));
        }
        
        void RenderService::renderCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            m_primitiveRenderer->renderCircle(color, position, normal, segments, radius, startAxis, endAxis);
        }
        
        void RenderService::renderCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            m_primitiveRenderer->renderCircle(color, position, normal, segments, radius, startAngle, angleLength);
        }
        
        void RenderService::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const Vec3f& startAxis, const Vec3f& endAxis) {
            m_primitiveRenderer->renderFilledCircle(color, position, normal, segments, radius, startAxis, endAxis);
        }
        
        void RenderService::renderFilledCircle(const Color& color, const Vec3f& position, const Math::Axis::Type normal, const size_t segments, const float radius, const float startAngle, const float angleLength) {
            m_primitiveRenderer->renderFilledCircle(color, position, normal, segments, radius, startAngle, angleLength);
        }
        
        void RenderService::render(RenderBatch& renderBatch) {
            renderBatch.add(m_primitiveRenderer);
            renderBatch.add(m_pointHandleRenderer);
        }
    }
}
