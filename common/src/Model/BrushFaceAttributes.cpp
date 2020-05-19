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
        const std::string BrushFaceAttributes::NoTextureName = "__TB_empty";

        BrushFaceAttributes::BrushFaceAttributes(const std::string& textureName) :
        m_textureName(textureName),
        m_offset(vm::vec2f::zero()),
        m_scale(vm::vec2f(1.0f, 1.0f)),
        m_rotation(0.0f),
        m_surfaceContents(0),
        m_surfaceFlags(0),
        m_surfaceValue(0.0f) {}

        BrushFaceAttributes::BrushFaceAttributes(const BrushFaceAttributes& other) :
        m_textureName(other.m_textureName),
        m_offset(other.m_offset),
        m_scale(other.m_scale),
        m_rotation(other.m_rotation),
        m_surfaceContents(other.m_surfaceContents),
        m_surfaceFlags(other.m_surfaceFlags),
        m_surfaceValue(other.m_surfaceValue),
        m_color(other.m_color) {}

        BrushFaceAttributes::BrushFaceAttributes(const std::string& textureName, const BrushFaceAttributes& other) :
        m_textureName(textureName),
        m_offset(other.m_offset),
        m_scale(other.m_scale),
        m_rotation(other.m_rotation),
        m_surfaceContents(other.m_surfaceContents),
        m_surfaceFlags(other.m_surfaceFlags),
        m_surfaceValue(other.m_surfaceValue),
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
                    lhs.m_surfaceContents == rhs.m_surfaceContents &&
                    lhs.m_surfaceFlags == rhs.m_surfaceFlags &&
                    lhs.m_surfaceValue == rhs.m_surfaceValue &&
                    lhs.m_color == rhs.m_color);
        }

        void swap(BrushFaceAttributes& lhs, BrushFaceAttributes& rhs) {
            using std::swap;
            swap(lhs.m_textureName, rhs.m_textureName);
            swap(lhs.m_offset, rhs.m_offset);
            swap(lhs.m_scale, rhs.m_scale);
            swap(lhs.m_rotation, rhs.m_rotation);
            swap(lhs.m_surfaceContents, rhs.m_surfaceContents);
            swap(lhs.m_surfaceFlags, rhs.m_surfaceFlags);
            swap(lhs.m_surfaceValue, rhs.m_surfaceValue);
            swap(lhs.m_color, rhs.m_color);
        }

        BrushFaceAttributes BrushFaceAttributes::takeSnapshot() const {
            BrushFaceAttributes result(m_textureName);
            result.m_offset = m_offset;
            result.m_scale = m_scale;
            result.m_rotation = m_rotation;
            result.m_surfaceContents = m_surfaceContents;
            result.m_surfaceFlags = m_surfaceFlags;
            result.m_surfaceValue = m_surfaceValue;
            result.m_color = m_color;
            return result;
        }

        const std::string& BrushFaceAttributes::textureName() const {
            return m_textureName;
        }

        void BrushFaceAttributes::setTextureName(const std::string& textureName) {
            m_textureName = textureName;
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

        int BrushFaceAttributes::surfaceContents() const {
            return m_surfaceContents;
        }

        int BrushFaceAttributes::surfaceFlags() const {
            return m_surfaceFlags;
        }

        float BrushFaceAttributes::surfaceValue() const {
            return m_surfaceValue;
        }

        bool BrushFaceAttributes::valid() const {
            return !vm::is_zero(m_scale.x(), vm::Cf::almost_zero()) && !vm::is_zero(m_scale.y(), vm::Cf::almost_zero());
        }

        void BrushFaceAttributes::setOffset(const vm::vec2f& offset) {
            m_offset = offset;
        }

        void BrushFaceAttributes::setXOffset(const float xOffset) {
            m_offset[0] = xOffset;
        }

        void BrushFaceAttributes::setYOffset(const float yOffset) {
            m_offset[1] = yOffset;
        }

        void BrushFaceAttributes::setScale(const vm::vec2f& scale) {
            m_scale = scale;
        }

        void BrushFaceAttributes::setXScale(const float xScale) {
            m_scale[0] = xScale;
        }

        void BrushFaceAttributes::setYScale(const float yScale) {
            m_scale[1] = yScale;
        }

        void BrushFaceAttributes::setRotation(const float rotation) {
            m_rotation = rotation;
        }

        void BrushFaceAttributes::setSurfaceContents(const int surfaceContents) {
            m_surfaceContents = surfaceContents;
        }

        void BrushFaceAttributes::setSurfaceFlags(const int surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
        }

        void BrushFaceAttributes::setSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
        }

        const Color& BrushFaceAttributes::color() const {
            return m_color;
        }

        void BrushFaceAttributes::setColor(const Color& color) {
            m_color = color;
        }
    }
}
