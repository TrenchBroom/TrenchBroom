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

#ifndef __TrenchBroom__BoxInfoRenderer__
#define __TrenchBroom__BoxInfoRenderer__

#include "Renderer/Text/TextRenderer.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class StringManager;
        }
        
        class Camera;
        class RenderContext;
        class Vbo;
        
        class BoxInfoTextAnchor : public Text::TextAnchor {
        private:
            const BBox m_bounds;
            const Axis::Type m_axis;
            const Renderer::Camera& m_camera;
        public:
            BoxInfoTextAnchor(const BBox& bounds, const Axis::Type axis, const Renderer::Camera& camera);
            const Vec3f position();
            Text::Alignment::Type alignment();
        };
        
        class BoxInfoTextFilter : public Text::TextRenderer<Axis::Type>::TextRendererFilter {
        public:
            bool stringVisible(RenderContext& context, const Axis::Type& key) {
                return true;
            }
        };
        
        class BoxInfoRenderer {
        private:
            BBox m_bounds;
            Text::TextRenderer<Axis::Type>* m_textRenderer;
            BoxInfoTextFilter m_textFilter;
            bool m_initialized;
        public:
            BoxInfoRenderer(const BBox& bounds, Text::StringManager& stringManager);
            ~BoxInfoRenderer();
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__BoxInfoRenderer__) */
