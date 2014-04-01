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

#ifndef __TrenchBroom__TexCoordSystemHelper__
#define __TrenchBroom__TexCoordSystemHelper__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        
        class TexCoordSystemHelper {
        private:
            typedef char Mode;
            static const Mode Default    = 0;
            static const Mode Translate  = 1 << 0;
            static const Mode Scale      = 1 << 1;
            static const Mode Project    = 1 << 2;
            
            const BrushFace* m_face;
            bool m_overrideOffset;
            bool m_overrideScale;
            Vec2f m_offset;
            Vec2f m_scale;
            Mode m_mode;
        public:
            TexCoordSystemHelper(const BrushFace* face);
            
            void setTranslate(bool translate = true);
            void setOverrideTranslate(const Vec2f& offset);
            void setScale(bool scale = true);
            void setOverrideScale(const Vec2f& scale);
            void setProject(bool project = true);
        private:
            void toggleMode(Mode mode, bool on);
        public:
            Vec3 worldToTex(const Vec3& v) const;
            Vec3::List worldToTex(const Vec3::List& vs) const;
            
            Vec3 texToWorld(const Vec3& v) const;
            Vec3::List texToWorld(const Vec3::List& vs) const;
            
            Vec3 texToTex(const Vec3& v, const TexCoordSystemHelper& other) const;
            Vec3::List texToTex(const Vec3::List& vs, const TexCoordSystemHelper& other) const;
            
            Mat4x4 toTexMatrix() const;
            Mat4x4 toWorldMatrix() const;
        private:
            Mat4x4 toTexMatrix(bool project) const;
            Mat4x4 toWorldMatrix(bool project) const;

            const Vec2f& scale() const;
            const Vec2f& offset() const;
            bool project() const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexCoordSystemHelper__) */
