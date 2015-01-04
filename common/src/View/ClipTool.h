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

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace View {
        class ClipTool : public Tool {
        public:
            class ClipPlaneStrategy {
            public:
                virtual ~ClipPlaneStrategy();
                
                bool computeClipPlane(const Vec3& point1, const Vec3& point2, Plane3& clipPlane) const;
                bool computeClipPlane(const Vec3& point1, const Vec3& point2, const Vec3& point3, Plane3& clipPlane) const;
            private:
                virtual bool doComputeClipPlane(const Vec3& point1, const Vec3& point2, Plane3& clipPlane) const = 0;
                virtual bool doComputeClipPlane(const Vec3& point1, const Vec3& point2, const Vec3& point3, Plane3& clipPlane) const = 0;
            };
        private:
            bool doActivate();
            bool doDeactivate();
        public:
            void pick(const Ray3& pickRay, Model::PickResult& pickResult);
            
            bool addClipPoint(const Vec3& point, const ClipPlaneStrategy& strategy);
            bool updateClipPoint(size_t index, const Vec3& newPosition, const ClipPlaneStrategy& strategy);
            void deleteLastClipPoint();
            
            bool reset();
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
