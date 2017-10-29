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

#ifndef TrenchBroom_Lasso
#define TrenchBroom_Lasso

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class Lasso {
        private:
            const Renderer::Camera& m_camera;
            const FloatType m_distance;
            const Mat4x4 m_transform;
            const Vec3 m_start;
            Vec3 m_cur;
        public:
            Lasso(const Renderer::Camera& camera, FloatType distance, const Vec3& point);
            
            void update(const Vec3& point);
            
            template <typename I, typename O>
            void selected(I cur, I end, O out) const {
                const Plane3 plane = this->plane();
                const BBox2 box = this->box();
                while (cur != end) {
                    if (selects(*cur, plane, box))
                        out = *cur;
                    ++cur;
                }
            }
            
            template <typename H>
            bool selects(const H& h) const {
                return selects(h, plane(), box());
            }
        private:
            bool selects(const Vec3& point, const Plane3& plane, const BBox2& box) const;
            bool selects(const Edge3& edge, const Plane3& plane, const BBox2& box) const;
            bool selects(const Polygon3& polygon, const Plane3& plane, const BBox2& box) const;
            Vec3 project(const Vec3& point, const Plane3& plane) const;
        public:
            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
        private:
            Plane3 plane() const;
            BBox2 box() const;
        };
    }
}

#endif /* defined(TrenchBroom_Lasso) */
