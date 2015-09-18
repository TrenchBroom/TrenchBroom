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

#ifndef TrenchBroom_BrushFaceAttributes
#define TrenchBroom_BrushFaceAttributes

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class BrushFaceAttributes {
        private:
            String m_textureName;
            Assets::Texture* m_texture;
            
            Vec2f m_offset;
            Vec2f m_scale;
            float m_rotation;
            
            int m_surfaceContents;
            int m_surfaceFlags;
            float m_surfaceValue;
        public:
            BrushFaceAttributes(const String& textureName);
            
            const String& textureName() const;
            Assets::Texture* texture() const;
            Vec2f textureSize() const;
            
            const Vec2f& offset() const;
            float xOffset() const;
            float yOffset() const;
            Vec2f modOffset(const Vec2f& offset) const;
            
            const Vec2f& scale() const;
            float xScale() const;
            float yScale() const;
            
            float rotation() const;
            
            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            
            void setTexture(Assets::Texture* texture);
            void setOffset(const Vec2f& offset);
            void setXOffset(float xOffset);
            void setYOffset(float yOffset);
            void setScale(const Vec2f& scale);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void setRotation(float rotation);
            void setSurfaceContents(int surfaceContents);
            void setSurfaceFlags(int surfaceFlags);
            void setSurfaceValue(float surfaceValue);
        };
    }
}

#endif /* defined(TrenchBroom_BrushFaceAttributes) */
