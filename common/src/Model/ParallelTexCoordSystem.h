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

#ifndef TrenchBroom_ParallelTexCoordSystem
#define TrenchBroom_ParallelTexCoordSystem

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/TexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        class ParallelTexCoordSystem;
        
        class ParallelTexCoordSystemSnapshot : public TexCoordSystemSnapshot {
        private:
            vec3 m_xAxis;
            vec3 m_yAxis;
        public:
            ParallelTexCoordSystemSnapshot(const vec3& xAxis, const vec3& yAxis);
            ParallelTexCoordSystemSnapshot(ParallelTexCoordSystem* coordSystem);
        private:
            TexCoordSystemSnapshot* doClone() const override;
            void doRestore(ParallelTexCoordSystem* coordSystem) const override;
            void doRestore(ParaxialTexCoordSystem* coordSystem) const override;
        };
        
        class ParallelTexCoordSystem : public TexCoordSystem {
        private:
            vec3 m_xAxis;
            vec3 m_yAxis;
            
            friend class ParallelTexCoordSystemSnapshot;
        public:
            ParallelTexCoordSystem(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs);
            ParallelTexCoordSystem(const vec3& xAxis, const vec3& yAxis);
        private:
            TexCoordSystem* doClone() const override;
            TexCoordSystemSnapshot* doTakeSnapshot() override;
            void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) override;
            
            vec3 getXAxis() const override;
            vec3 getYAxis() const override;
            vec3 getZAxis() const override;

            void doResetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) override;
            void doResetTextureAxes(const vec3& normal) override;
            void doResetTextureAxesToParaxial(const vec3& normal, float angle) override;
            void doResetTextureAxesToParallel(const vec3& normal, float angle) override;

            bool isRotationInverted(const vec3& normal) const override;
            vec2f doGetTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const override;
            
            void doSetRotation(const vec3& normal, float oldAngle, float newAngle) override;
            void applyRotation(const vec3& normal, FloatType angle);
            
            void doTransform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& invariant) override;
            float computeTextureAngle(const plane3& oldBoundary, const mat4x4& transformation) const;
            mat4x4 computeNonTextureRotation(const vec3& oldNormal, const vec3& newNormal, const mat4x4& rotation) const;
            
            void doUpdateNormalWithProjection(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) override;
            void doUpdateNormalWithRotation(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) override;

            void doShearTexture(const vec3& normal, const vec2f& factors) override;

            float doMeasureAngle(float currentAngle, const vec2f& center, const vec2f& point) const override;
            void computeInitialAxes(const vec3& normal, vec3& xAxis, vec3& yAxis) const;
        private:
            ParallelTexCoordSystem(const ParallelTexCoordSystem& other);
            ParallelTexCoordSystem& operator=(const ParallelTexCoordSystem& other);
        };
    }
}

#endif /* defined(TrenchBroom_ParallelTexCoordSystem) */
