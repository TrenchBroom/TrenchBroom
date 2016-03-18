/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "TextAnchor.h"

#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace Renderer {
        TextAnchor::~TextAnchor() {}
        
        TextAnchor3D::~TextAnchor3D() {}
        
        Vec3f TextAnchor3D::offset(const Camera& camera, const Vec2f& size) const {
            const Vec2f halfSize = size / 2.0f;
            const TextAlignment::Type a = alignment();
            const Vec2f factors = alignmentFactors(a);
            const Vec2f extra = extraOffsets(a, size);
            Vec3f offset = camera.project(basePosition());
            for (size_t i = 0; i < 2; i++)
                offset[i] = Math::round(offset[i] + factors[i] * size[i] - halfSize[i] + extra[i]);
            return offset;
        }
        
        Vec3f TextAnchor3D::position(const Camera& camera) const {
            return basePosition();
        }
        
        Vec2f TextAnchor3D::alignmentFactors(const TextAlignment::Type a) const {
            Vec2f factors;
            if ((a & TextAlignment::Left))
                factors[0] = +0.5f;
            else if ((a & TextAlignment::Right))
                factors[0] = -0.5f;
            if ((a & TextAlignment::Top))
                factors[1] = -0.5f;
            else if ((a & TextAlignment::Bottom))
                factors[1] = +0.5f;
            return factors;
        }
        
        Vec2f TextAnchor3D::extraOffsets(const TextAlignment::Type a, const Vec2f& size) const {
            return Vec2f::Null;
        }
        
        Vec3f SimpleTextAnchor::basePosition() const {
            return m_position;
        }
        
        TextAlignment::Type SimpleTextAnchor::alignment() const {
            return m_alignment;
        }
        
        Vec2f SimpleTextAnchor::extraOffsets(const TextAlignment::Type a, const Vec2f& size) const {
            return m_extraOffsets;
        }
        
        SimpleTextAnchor::SimpleTextAnchor(const Vec3f& position, const TextAlignment::Type alignment, const Vec2f& extraOffsets) :
        m_position(position),
        m_alignment(alignment),
        m_extraOffsets(extraOffsets) {}
    }
}
