/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__PointHandleRenderer__
#define __TrenchBroom__PointHandleRenderer__

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class InstancedVertexArray;
        class RenderContext;
        class Vbo;
        class VertexArray;
        
        class PointHandleRenderer {
        private:
            float m_radius;
            unsigned int m_iterations;
            float m_scalingFactor;
            float m_maximumDistance;

            Color m_color;
            Vec4f::List m_positions;
            bool m_valid;
        protected:
            Vec3f::List sphere() const;
            
            inline float scalingFactor() const {
                return m_scalingFactor;
            }
            
            inline float maximumDistance() const {
                return m_maximumDistance;
            }
            
            inline const Vec4f::List& positions() const {
                return m_positions;
            }
            
            inline bool valid() const {
                return m_valid;
            }

            inline void validate() {
                m_valid = true;
            }
            
            inline const Color& color() const {
                return m_color;
            }
        public:
            PointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance) :
            m_radius(radius),
            m_iterations(iterations),
            m_scalingFactor(scalingFactor),
            m_maximumDistance(maximumDistance),
            m_valid(false) {}
            
            virtual ~PointHandleRenderer() {}
            
            static bool instancingSupported();
            static PointHandleRenderer* create(float radius, unsigned int iterations, float scalingFactor, float maximumDistance);
            
            inline void add(const Vec3f& position) {
                m_positions.push_back(Vec4f(position.x, position.y, position.z, 0.0f));
                m_valid = false;
            }
            
            inline void clear() {
                m_valid &= m_positions.empty();
                m_positions.clear();
            }
            
            inline void setColor(const Color& color) {
                m_color = color;
            }
            virtual void render(Vbo& vbo, RenderContext& context) = 0;
        };
        
        class DefaultPointHandleRenderer : public PointHandleRenderer {
        protected:
            VertexArray* m_vertexArray;
        public:
            DefaultPointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance);
            ~DefaultPointHandleRenderer();
            
            void render(Vbo& vbo, RenderContext& context);
        };
        
        class InstancedPointHandleRenderer : public PointHandleRenderer {
        protected:
            InstancedVertexArray* m_vertexArray;
        public:
            InstancedPointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance);
            ~InstancedPointHandleRenderer();

            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__PointHandleRenderer__) */
