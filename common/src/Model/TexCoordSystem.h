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

#ifndef TrenchBroom_TexCoordSystem
#define TrenchBroom_TexCoordSystem

#include "VecMath.h"
#include "TrenchBroom.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class BrushFaceAttributes;
        class TexCoordSystem;
        class ParallelTexCoordSystem;
        class ParaxialTexCoordSystem;
        
        class TexCoordSystemSnapshot {
        public:
            virtual ~TexCoordSystemSnapshot();
            void restore(TexCoordSystem* coordSystem) const;
            TexCoordSystemSnapshot* clone() const;
        private:
            virtual TexCoordSystemSnapshot* doClone() const = 0;
            virtual void doRestore(ParallelTexCoordSystem* coordSystem) const = 0;
            virtual void doRestore(ParaxialTexCoordSystem* coordSystem) const = 0;
            
            friend class ParallelTexCoordSystem;
            friend class ParaxialTexCoordSystem;
        };
        
        enum class WrapStyle {
        	Projection,
            Rotation
        };
        
        class TexCoordSystem {
        public:
            TexCoordSystem();
            virtual ~TexCoordSystem();
            
            TexCoordSystem* clone() const;
            TexCoordSystemSnapshot* takeSnapshot();
            
            vec3 xAxis() const;
            vec3 yAxis() const;

            void resetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs);
            void resetTextureAxes(const vec3& normal);
            void resetTextureAxesToParaxial(const vec3& normal, float angle);
            void resetTextureAxesToParallel(const vec3& normal, float angle);
            
            vec2f getTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const;
            
            void setRotation(const vec3& normal, float oldAngle, float newAngle);
            void transform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& invariant);
            void updateNormal(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs, const WrapStyle style);

            void moveTexture(const vec3& normal, const vec3& up, const vec3& right, const vec2f& offset, BrushFaceAttributes& attribs) const;
            void rotateTexture(const vec3& normal, float angle, BrushFaceAttributes& attribs) const;
            void shearTexture(const vec3& normal, const vec2f& factors);

            mat4x4 toMatrix(const vec2f& offset, const vec2f& scale) const;
            mat4x4 fromMatrix(const vec2f& offset, const vec2f& scale) const;
            float measureAngle(float currentAngle, const vec2f& center, const vec2f& point) const;
        private:
            virtual TexCoordSystem* doClone() const = 0;
            virtual TexCoordSystemSnapshot* doTakeSnapshot() = 0;
            virtual void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) = 0;
            friend class TexCoordSystemSnapshot;
            
            virtual vec3 getXAxis() const = 0;
            virtual vec3 getYAxis() const = 0;
            virtual vec3 getZAxis() const = 0;

            virtual void doResetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) = 0;
            virtual void doResetTextureAxes(const vec3& normal) = 0;
            virtual void doResetTextureAxesToParaxial(const vec3& normal, float angle) = 0;
            virtual void doResetTextureAxesToParallel(const vec3& normal, float angle) = 0;

            virtual bool isRotationInverted(const vec3& normal) const = 0;
            virtual vec2f doGetTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const = 0;
            
            virtual void doSetRotation(const vec3& normal, float oldAngle, float newAngle) = 0;
            virtual void doTransform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& invariant) = 0;
            virtual void doUpdateNormalWithProjection(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) = 0;
            virtual void doUpdateNormalWithRotation(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) = 0;

            virtual void doShearTexture(const vec3& normal, const vec2f& factors) = 0;
            
            virtual float doMeasureAngle(float currentAngle, const vec2f& center, const vec2f& point) const = 0;
        protected:
            vec2f computeTexCoords(const vec3& point, const vec2f& scale) const;

            template <typename T>
            T safeScale(const T value) const {
                return Math::zero(value) ? static_cast<T>(1.0) : value;
            }
            
            template <typename T1, typename T2>
            vec<T1,3> safeScaleAxis(const vec<T1,3>& axis, const T2 factor) const {
                return axis / safeScale(T1(factor));
            }
        private:
            TexCoordSystem(const TexCoordSystem& other);
            TexCoordSystem& operator=(const TexCoordSystem& other);
        };
    }
}

#endif /* defined(TrenchBroom_TexCoordSystem) */
