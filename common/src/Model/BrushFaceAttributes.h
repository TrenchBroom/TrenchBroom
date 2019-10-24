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

#ifndef TrenchBroom_BrushFaceAttributes
#define TrenchBroom_BrushFaceAttributes

#include "StringType.h"
#include "Color.h"

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushFaceAttributes {
        private:
            String m_textureName;
            Assets::Texture* m_texture;

            vm::vec2f m_offset;
            vm::vec2f m_scale;
            float m_rotation;

            int m_surfaceContents;
            int m_surfaceFlags;
            float m_surfaceValue;

            Color m_color;
        public:
            BrushFaceAttributes(const String& textureName);
            BrushFaceAttributes(const BrushFaceAttributes& other);
            ~BrushFaceAttributes();
            BrushFaceAttributes& operator=(BrushFaceAttributes other);
            friend void swap(BrushFaceAttributes& lhs, BrushFaceAttributes& rhs);

            BrushFaceAttributes takeSnapshot() const;

            const String& textureName() const;
            Assets::Texture* texture() const;
            vm::vec2f textureSize() const;

            const vm::vec2f& offset() const;
            float xOffset() const;
            float yOffset() const;
            vm::vec2f modOffset(const vm::vec2f& offset) const;

            const vm::vec2f& scale() const;
            float xScale() const;
            float yScale() const;

            float rotation() const;

            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;

            void setTexture(Assets::Texture* texture);
            void unsetTexture();

            bool valid() const;

            void setOffset(const vm::vec2f& offset);
            void setXOffset(float xOffset);
            void setYOffset(float yOffset);
            void setScale(const vm::vec2f& scale);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void setRotation(float rotation);
            void setSurfaceContents(int surfaceContents);
            void setSurfaceFlags(int surfaceFlags);
            void setSurfaceValue(float surfaceValue);

            const Color& color() const;
            void setColor(const Color& color);
        };
    }
}

#endif /* defined(TrenchBroom_BrushFaceAttributes) */
