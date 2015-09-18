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

#ifndef TrenchBroom_TextAnchor
#define TrenchBroom_TextAnchor

#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        
        namespace TextAlignment {
            typedef unsigned int Type;
            static const Type Top       = 1 << 0;
            static const Type Bottom    = 1 << 1;
            static const Type Left      = 1 << 2;
            static const Type Right     = 1 << 3;
            static const Type Center    = 1 << 4;
        }
        
        class TextAnchor {
        public:
            virtual ~TextAnchor() {}
            Vec3f offset(const Camera& camera, const Vec2f& size) const;
            Vec3f position() const;
        private:
            Vec2f alignmentFactors(TextAlignment::Type a) const;
        private:
            virtual Vec3f basePosition() const = 0;
            virtual TextAlignment::Type alignment() const = 0;
            virtual Vec2f extraOffsets(TextAlignment::Type a, const Vec2f& size) const;
        };
        
        class SimpleTextAnchor : public TextAnchor {
        private:
            Vec3f m_position;
            TextAlignment::Type m_alignment;
            Vec2f m_extraOffsets;
        public:
            SimpleTextAnchor(const Vec3f& position, const TextAlignment::Type alignment, const Vec2f& extraOffsets = Vec2f::Null);
        private:
            Vec3f basePosition() const;
            TextAlignment::Type alignment() const;
            Vec2f extraOffsets(TextAlignment::Type a, const Vec2f& size) const;
        };
    }
}

#endif /* defined(TrenchBroom_TextAnchor) */
