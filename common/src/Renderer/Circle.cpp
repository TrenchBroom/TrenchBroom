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

#include "Circle.h"

#include "Renderer/PrimType.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"

#include <vecmath/forward.h>
#include <vecmath/constants.h>
#include <vecmath/vec.h>
#include <vecmath/util.h>

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Circle::Circle(const float radius, const size_t segments, const bool filled) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, 0.0f, vm::Cf::two_pi());
        }

        Circle::Circle(const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, startAngle, angleLength);
        }

        Circle::Circle(const float radius, const size_t segments, const bool filled, const vm::axis::type axis, const vm::vec3f& startAxis, const vm::vec3f& endAxis) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);

            const std::pair<float, float> angles = startAngleAndLength(axis, startAxis, endAxis);
            init3D(radius, segments, axis, angles.first, angles.second);
        }

        Circle::Circle(const float radius, const size_t segments, const bool filled, const vm::axis::type axis, const float startAngle, const float angleLength) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            assert(angleLength > 0.0f);
            init3D(radius, segments, axis, startAngle, angleLength);
        }

        bool Circle::prepared() const {
            return m_array.prepared();
        }

        void Circle::prepare(VboManager& vboManager) {
            m_array.prepare(vboManager);
        }

        void Circle::render() {
            m_array.render(m_filled ? PrimType::TriangleFan : PrimType::LineLoop);
        }

        void Circle::init2D(const float radius, const size_t segments, const float startAngle, const float angleLength) {
            using Vertex = GLVertexTypes::P2::Vertex;

            auto positions = circle2D(radius, startAngle, angleLength, segments);
            if (m_filled) {
                positions.push_back(vm::vec2f::zero());
            }
            m_array = VertexArray::move(Vertex::toList(positions.size(), std::begin(positions)));
        }

        void Circle::init3D(const float radius, const size_t segments, const vm::axis::type axis, const float startAngle, const float angleLength) {
            using Vertex = GLVertexTypes::P3::Vertex;

            auto positions = circle2D(radius, axis, startAngle, angleLength, segments);
            if (m_filled) {
                positions.emplace_back(vm::vec3f::zero());
            }
            m_array = VertexArray::move(Vertex::toList(positions.size(), std::begin(positions)));
        }
    }
}
