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

#include "FontGlyph.h"

namespace TrenchBroom {
    namespace Renderer {
        FontGlyph::FontGlyph(const float x, const float y, const float w, const float h, const int a) :
        m_x(x),
        m_y(y),
        m_w(w),
        m_h(h),
        m_a(a) {}
        
        void FontGlyph::appendVertices(Vec2f::List& vertices, const float xOffset, const float yOffset, const float textureSize, const bool clockwise) const {
            if (clockwise) {
                vertices.push_back(Vec2f(xOffset, yOffset));
                vertices.push_back(Vec2f(m_x, m_y + m_h) / textureSize);
                
                vertices.push_back(Vec2f(xOffset, yOffset + m_h));
                vertices.push_back(Vec2f(m_x, m_y) / textureSize);
                
                vertices.push_back(Vec2f(xOffset + m_w, yOffset + m_h));
                vertices.push_back(Vec2f(m_x + m_w, m_y) / textureSize);
                
                vertices.push_back(Vec2f(xOffset + m_w, yOffset));
                vertices.push_back(Vec2f(m_x + m_w, m_y + m_h) / textureSize);
            } else {
                vertices.push_back(Vec2f(xOffset, yOffset));
                vertices.push_back(Vec2f(m_x, m_y + m_h) / textureSize);
                
                vertices.push_back(Vec2f(xOffset + m_w, yOffset));
                vertices.push_back(Vec2f(m_x + m_w, m_y + m_h) / textureSize);
                
                vertices.push_back(Vec2f(xOffset + m_w, yOffset + m_h));
                vertices.push_back(Vec2f(m_x + m_w, m_y) / textureSize);
                
                vertices.push_back(Vec2f(xOffset, yOffset + m_h));
                vertices.push_back(Vec2f(m_x, m_y) / textureSize);
            }
        }

        int FontGlyph::advance() const {
            return m_a;
        }

        float FontGlyph::sMin(const float textureSize) const {
            return m_x / textureSize;
        }
        
        float FontGlyph::sMax(const float textureSize) const {
            return m_x + m_w / textureSize;
        }
        
        float FontGlyph::tMin(const float textureSize) const {
            return m_y / textureSize;
        }
        
        float FontGlyph::tMax(const float textureSize) const {
            return m_y + m_h / textureSize;
        }
    }
}
