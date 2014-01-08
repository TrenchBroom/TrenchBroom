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

#include "TextRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        const Vec3f TextAnchor::offset(const Camera& camera, const Vec2f& size) const {
            const Vec2f halfSize = size / 2.0f;
            const Vec2f factors = alignmentFactors();
            const Vec2f extra = extraOffsets();
            Vec3f offset = camera.project(basePosition());
            for (size_t i = 0; i < 2; i++)
                offset[i] = Math::round(offset[i] + factors[i] * size[i] - halfSize[i] + extra[i]);
            return offset;
        }
        
        const Vec3f TextAnchor::position() const {
            return basePosition();
        }

        const Vec2f TextAnchor::extraOffsets() const {
            return Vec2f::Null;
        }

        const Vec2f TextAnchor::alignmentFactors() const {
            const Alignment::Type a = alignment();
            Vec2f factors;
            if ((a & Alignment::Left))
                factors[0] = +0.5f;
            else if ((a & Alignment::Right))
                factors[0] = -0.5f;
            if ((a & Alignment::Top))
                factors[1] = -0.5f;
            else if ((a & Alignment::Bottom))
                factors[1] = +0.5f;
            return factors;
        }

        const Vec3f SimpleTextAnchor::basePosition() const {
            return m_position;
        }
        
        const Alignment::Type SimpleTextAnchor::alignment() const {
            return m_alignment;
        }

        const Vec2f SimpleTextAnchor::extraOffsets() const {
            return m_extraOffsets;
        }

        SimpleTextAnchor::SimpleTextAnchor(const Vec3f& position, const Alignment::Type alignment, const Vec2f& extraOffsets) :
        m_position(position),
        m_alignment(alignment),
        m_extraOffsets(extraOffsets) {}
    }
}
