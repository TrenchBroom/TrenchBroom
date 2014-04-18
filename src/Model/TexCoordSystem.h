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

#ifndef __TrenchBroom__TexCoordSystem__
#define __TrenchBroom__TexCoordSystem__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class BrushFaceAttribs;
        
        class TexCoordSystem {
        public:
            virtual ~TexCoordSystem();
            
            TexCoordSystem* clone() const;
            
            Vec2f getTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const;
            void update(const Vec3& normal, const BrushFaceAttribs& attribs);
            void compensate(const Vec3& normal, const Vec3& center, const Mat4x4& transformation, BrushFaceAttribs& attribs);
            
            void moveTexture(const Vec3& normal, const Vec3& up, const Vec3& right, const Math::Direction direction, float distance, BrushFaceAttribs& attribs) const;
            void rotateTexture(const Vec3& normal, float angle, BrushFaceAttribs& attribs) const;

            Mat4x4 toMatrix(const Vec2f& offset, const Vec2f& scale) const;
            Mat4x4 fromMatrix(const Vec2f& offset, const Vec2f& scale) const;
            float measureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const;
        protected:
            Vec3 project(const Vec3& normal, const Vec3& vec) const;
            Vec3 project(const Plane3& plane, const Vec3& vec) const;
        private:
            virtual TexCoordSystem* doClone() const = 0;

            virtual const Vec3& getXAxis() const = 0;
            virtual const Vec3& getYAxis() const = 0;
            virtual const Vec3& getZAxis() const = 0;
            virtual bool isRotationInverted(const Vec3& normal) const = 0;
            
            virtual Vec2f doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const = 0;
            virtual void doUpdate(const Vec3& normal, const BrushFaceAttribs& attribs) = 0;
            virtual void doCompensate(const Vec3& normal, const Vec3& center, const Mat4x4& transformation, BrushFaceAttribs& attribs) = 0;

            virtual float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__TexCoordSystem__) */
