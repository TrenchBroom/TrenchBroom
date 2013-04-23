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

#ifndef __TrenchBroom__BoxGuideRenderer__
#define __TrenchBroom__BoxGuideRenderer__

#include "Model/Filter.h"
#include "Renderer/BoxInfoRenderer.h"
#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Picker;
    }
    
    namespace Renderer {
        namespace Text {
            class FontManager;
        }
        
        class RenderContext;
        class VertexArray;
        class Vbo;
        
        class BoxGuideRenderer {
            BoxInfoRenderer m_infoRenderer;
            Color m_color;
            BBoxf m_bounds;
            Model::Picker& m_picker;
            Model::VisibleFilter m_filter;
            VertexArray* m_boxArray;
            VertexArray* m_spikeArray;
            VertexArray* m_pointArray;
            bool m_showSizes;
            bool m_valid;
            
            void addSpike(const Vec3f& startPoint, const Vec3f& direction, Vec3f::List& hitPoints);
        public:
            BoxGuideRenderer(const BBoxf& bounds, Model::Picker& picker, Model::Filter& defaultFilter, Text::FontManager& fontManager);
            ~BoxGuideRenderer();
            
            inline const BBoxf& bounds() const {
                return m_bounds;
            }
            
            inline void setColor(const Color& color) {
                if (color == m_color)
                    return;
                m_color = color;
                m_valid = false;
            }
            
            inline void setShowSizes(bool showSizes) {
                m_showSizes = showSizes;
            }
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__BoxGuideRenderer__) */
