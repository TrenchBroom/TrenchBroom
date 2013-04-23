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

#ifndef __TrenchBroom__PointGuideRenderer__
#define __TrenchBroom__PointGuideRenderer__

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Filter;
        class Picker;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VertexArray;
        
        class PointGuideRenderer {
        protected:
            Color m_color;
            Vec3f m_position;
            Model::Picker& m_picker;
            Model::Filter& m_filter;
            VertexArray* m_spikeArray;
            VertexArray* m_pointArray;
            bool m_valid;

            void addSpike(const Vec3f& direction, Vec3f::List& hitPoints);
        public:
            PointGuideRenderer(const Vec3f& position, Model::Picker& picker, Model::Filter& defaultFilter);
            ~PointGuideRenderer();
            
            inline void setColor(const Color& color) {
                if (color == m_color)
                    return;
                m_color = color;
                m_valid = false;
            }
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__PointGuideRenderer__) */
