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

#include "BrushFaceAttributes.h"
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Model {
        BrushFaceAttributes::BrushFaceAttributes(const String& textureName) :
        m_textureName(textureName),
        m_texture(NULL),
        m_offset(Vec2f::Null),
        m_scale(Vec2f(1.0f, 1.0f)),
        m_rotation(0.0f),
        m_surfaceContents(0),
        m_surfaceFlags(0),
        m_surfaceValue(0.0f) {}
        
        const String& BrushFaceAttributes::textureName() const {
            return m_textureName;
        }
        
        Assets::Texture* BrushFaceAttributes::texture() const {
            return m_texture;
        }
        
        Vec2f BrushFaceAttributes::textureSize() const {
            if (m_texture == NULL)
                return Vec2f::One;
            const float w = m_texture->width()  == 0 ? 1.0f : static_cast<float>(m_texture->width());
            const float h = m_texture->height() == 0 ? 1.0f : static_cast<float>(m_texture->height());
            return Vec2f(w, h);
        }
        
        const Vec2f& BrushFaceAttributes::offset() const {
            return m_offset;
        }
        
        float BrushFaceAttributes::xOffset() const {
            return m_offset.x();
        }
        
        float BrushFaceAttributes::yOffset() const {
            return m_offset.y();
        }
        
        Vec2f BrushFaceAttributes::modOffset(const Vec2f& offset) const {
            return offset - offset.roundDownToMultiple(textureSize());
        }
        
        const Vec2f& BrushFaceAttributes::scale() const {
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
        
        void BrushFaceAttributes::setTexture(Assets::Texture* texture) {
            m_texture = texture;
            if (m_texture != NULL)
                m_textureName = texture->name();
        }
        
        void BrushFaceAttributes::setOffset(const Vec2f& offset) {
            m_offset = offset;
        }
        
        void BrushFaceAttributes::setXOffset(const float xOffset) {
            m_offset[0] = xOffset;
        }
        
        void BrushFaceAttributes::setYOffset(const float yOffset) {
            m_offset[1] = yOffset;
        }
        
        void BrushFaceAttributes::setScale(const Vec2f& scale) {
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
    }
}
