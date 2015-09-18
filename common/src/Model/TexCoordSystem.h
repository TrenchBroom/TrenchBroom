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

#ifndef TrenchBroom_TexCoordSystem
#define TrenchBroom_TexCoordSystem

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class BrushFaceAttributes;
        
        class TexCoordSystemSnapshot {
        public:
            virtual ~TexCoordSystemSnapshot();
            void restore();
        private:
            virtual void doRestore() = 0;
        };
        
        class TexCoordSystem {
        public:
            virtual ~TexCoordSystem();
            
            TexCoordSystem* clone() const;
            TexCoordSystemSnapshot* takeSnapshot();
            
            Vec3 xAxis() const;
            Vec3 yAxis() const;

            void resetTextureAxes(const Vec3& normal);
            void resetTextureAxesToParaxial(const Vec3& normal, float angle);
            void resetTextureAxesToParallel(const Vec3& normal, float angle);
            
            Vec2f getTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const;
            
            void setRotation(const Vec3& normal, float oldAngle, float newAngle);
            void transform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant);
            void updateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs);
            
            void moveTexture(const Vec3& normal, const Vec3& up, const Vec3& right, const Vec2f& offset, BrushFaceAttributes& attribs) const;
            void rotateTexture(const Vec3& normal, float angle, BrushFaceAttributes& attribs) const;
            void shearTexture(const Vec3& normal, const Vec2f& factors);

            Mat4x4 toMatrix(const Vec2f& offset, const Vec2f& scale) const;
            Mat4x4 fromMatrix(const Vec2f& offset, const Vec2f& scale) const;
            float measureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const;
        private:
            virtual TexCoordSystem* doClone() const = 0;
            virtual TexCoordSystemSnapshot* doTakeSnapshot() = 0;
            
            virtual Vec3 getXAxis() const = 0;
            virtual Vec3 getYAxis() const = 0;
            virtual Vec3 getZAxis() const = 0;
            
            virtual void doResetTextureAxes(const Vec3& normal) = 0;
            virtual void doResetTextureAxesToParaxial(const Vec3& normal, float angle) = 0;
            virtual void doResetTextureAxesToParallel(const Vec3& normal, float angle) = 0;

            virtual bool isRotationInverted(const Vec3& normal) const = 0;
            virtual Vec2f doGetTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const = 0;
            
            virtual void doSetRotation(const Vec3& normal, float oldAngle, float newAngle) = 0;
            virtual void doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant) = 0;
            virtual void doUpdateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) = 0;

            virtual void doShearTexture(const Vec3& normal, const Vec2f& factors) = 0;
            
            virtual float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const = 0;
        protected:
            Vec2f computeTexCoords(const Vec3& point, const Vec2f& scale) const;

            template <typename T>
            T safeScale(const T value) const {
                return Math::zero(value) ? static_cast<T>(1.0) : value;
            }
            
            template <typename T1, typename T2>
            Vec<T1,3> safeScaleAxis(const Vec<T1,3>& axis, const T2 factor) const {
                return axis / safeScale(factor);
            }
        };
    }
}

#endif /* defined(TrenchBroom_TexCoordSystem) */
