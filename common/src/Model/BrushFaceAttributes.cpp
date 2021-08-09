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

#include "BrushFaceAttributes.h"
#include "Assets/Texture.h"

#include <vecmath/vec.h>

#include <string>

namespace TrenchBroom {
    namespace Model {
        bool SurfaceAttributes::operator==(const SurfaceAttributes& other) const {
            return surfaceContents == other.surfaceContents
                && surfaceFlags == other.surfaceFlags
                && surfaceValue == other.surfaceValue;
        }

        SurfaceAttributes SurfaceAttributes::makeContentsFlagsValue(const int i_surfaceContents, const int i_surfaceFlags, const float i_surfaceValue) {
            return {i_surfaceContents, i_surfaceFlags, i_surfaceValue};
        }

        const std::string BrushFaceAttributes::NoTextureName = "__TB_empty";

        BrushFaceAttributes::BrushFaceAttributes(std::string_view textureName) :
        m_textureName(textureName),
        m_offset(vm::vec2f::zero()),
        m_scale(vm::vec2f(1.0f, 1.0f)),
        m_rotation(0.0f) {}

        BrushFaceAttributes::BrushFaceAttributes(const BrushFaceAttributes& other) :
        m_textureName(other.m_textureName),
        m_offset(other.m_offset),
        m_scale(other.m_scale),
        m_rotation(other.m_rotation),
        m_surfaceAttributes(other.m_surfaceAttributes),
        m_color(other.m_color) {}

        BrushFaceAttributes::BrushFaceAttributes(std::string_view textureName, const BrushFaceAttributes& other) :
        m_textureName(textureName),
        m_offset(other.m_offset),
        m_scale(other.m_scale),
        m_rotation(other.m_rotation),
        m_surfaceAttributes(other.m_surfaceAttributes),
        m_color(other.m_color) {}

        BrushFaceAttributes& BrushFaceAttributes::operator=(BrushFaceAttributes other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        bool operator==(const BrushFaceAttributes& lhs, const BrushFaceAttributes& rhs) {
            return (lhs.m_textureName == rhs.m_textureName &&
                    lhs.m_offset == rhs.m_offset &&
                    lhs.m_scale == rhs.m_scale &&
                    lhs.m_rotation == rhs.m_rotation &&
                    lhs.m_surfaceAttributes == rhs.m_surfaceAttributes &&
                    lhs.m_color == rhs.m_color);
        }

        void swap(BrushFaceAttributes& lhs, BrushFaceAttributes& rhs) {
            using std::swap;
            swap(lhs.m_textureName, rhs.m_textureName);
            swap(lhs.m_offset, rhs.m_offset);
            swap(lhs.m_scale, rhs.m_scale);
            swap(lhs.m_rotation, rhs.m_rotation);
            swap(lhs.m_surfaceAttributes, rhs.m_surfaceAttributes);
            swap(lhs.m_color, rhs.m_color);
        }

        const std::string& BrushFaceAttributes::textureName() const {
            return m_textureName;
        }

        const vm::vec2f& BrushFaceAttributes::offset() const {
            return m_offset;
        }

        float BrushFaceAttributes::xOffset() const {
            return m_offset.x();
        }

        float BrushFaceAttributes::yOffset() const {
            return m_offset.y();
        }

        vm::vec2f BrushFaceAttributes::modOffset(const vm::vec2f& offset, const vm::vec2f& textureSize) const {
            return offset - snapDown(offset, textureSize);
        }

        const vm::vec2f& BrushFaceAttributes::scale() const {
            return m_scale;
        }

        float BrushFaceAttributes::xScale() const {
            return m_scale.x();
        }

        float BrushFaceAttributes::yScale() const {
            return m_scale.y();
        }

        float BrushFaceAttributes::rotation() const {
            return m_rotation;
        }

        bool BrushFaceAttributes::hasSurfaceAttributes() const {
            return m_surfaceAttributes.has_value();
        }

        const std::optional<SurfaceAttributes>& BrushFaceAttributes::surfaceAttributes() const {
            return m_surfaceAttributes;
        }

        bool BrushFaceAttributes::hasColor() const {
            return m_color.a() > 0.0f;
        }
        
        const Color& BrushFaceAttributes::color() const {
            return m_color;
        }

        bool BrushFaceAttributes::valid() const {
            return !vm::is_zero(m_scale.x(), vm::Cf::almost_zero()) && !vm::is_zero(m_scale.y(), vm::Cf::almost_zero());
        }
        
        bool BrushFaceAttributes::setTextureName(const std::string& textureName) {
            if (textureName == m_textureName) {
                return false;
            } else {
                m_textureName = textureName;
                return true;
            }
        }

        bool BrushFaceAttributes::setOffset(const vm::vec2f& offset) {
            if (offset == m_offset) {
                return false;
            } else {
                m_offset = offset;
                return true;
            }
        }

        bool BrushFaceAttributes::setXOffset(const float xOffset) {
            if (xOffset == m_offset.x()) {
                return false;
            } else {
                m_offset[0] = xOffset;
                return true;
            }
        }

        bool BrushFaceAttributes::setYOffset(const float yOffset) {
            if (yOffset == m_offset.y()) {
                return false;
            } else {
                m_offset[1] = yOffset;
                return true;
            }
        }

        bool BrushFaceAttributes::setScale(const vm::vec2f& scale) {
            if (scale == m_scale) {
                return false;
            } else {
                m_scale = scale;
                return true;
            }
        }

        bool BrushFaceAttributes::setXScale(const float xScale) {
            if (xScale == m_scale.x()) {
                return false;
            } else {
                m_scale[0] = xScale;
                return true;
            }
        }

        bool BrushFaceAttributes::setYScale(const float yScale) {
            if (yScale == m_scale.y()) {
                return false;
            } else {
                m_scale[1] = yScale;
                return true;
            }
        }

        bool BrushFaceAttributes::setRotation(const float rotation) {
            if (rotation == m_rotation) {
                return false;
            } else {
                m_rotation = rotation;
                return true;
            }
        }

        bool BrushFaceAttributes::setSurfaceAttributes(const std::optional<SurfaceAttributes>& surfaceAttributes) {
            if (surfaceAttributes == m_surfaceAttributes) {
                return false;
            } else {
                m_surfaceAttributes = surfaceAttributes;
                return true;
            }
        }

        bool BrushFaceAttributes::setColor(const Color& color) {
            if (color == m_color) {
                return false;
            } else {
                m_color = color;
                return true;
            }
        }
    }
}
