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
            const mat4x4 m_transform;
            const vm::vec3 m_start;
            vm::vec3 m_cur;
        public:
            Lasso(const Renderer::Camera& camera, FloatType distance, const vm::vec3& point);
            
            void update(const vm::vec3& point);
            
            template <typename I, typename O>
            void selected(I cur, I end, O out) const {
                const plane3 plane = this->plane();
                const bbox2 box = this->box();
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
            bool selects(const vm::vec3& point, const plane3& plane, const bbox2& box) const;
            bool selects(const segment3& edge, const plane3& plane, const bbox2& box) const;
            bool selects(const polygon3& polygon, const plane3& plane, const bbox2& box) const;
            vm::vec3 project(const vm::vec3& point, const plane3& plane) const;
        public:
            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
        private:
            plane3 plane() const;
            bbox2 box() const;
        };
    }
}

#endif /* defined(TrenchBroom_Lasso) */
