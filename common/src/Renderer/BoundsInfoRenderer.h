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

#ifndef __TrenchBroom__BoundsInfoRenderer__
#define __TrenchBroom__BoundsInfoRenderer__

#include "Color.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/TextRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderContext;
        
        class BoundsInfoRenderer {
        private:
            class BoundsInfoSizeTextAnchor : public TextAnchor {
            private:
                const BBox3& m_bounds;
                Math::Axis::Type m_axis;
                const Renderer::Camera& m_camera;
            public:
                BoundsInfoSizeTextAnchor(const BBox3& bounds, Math::Axis::Type axis, const Renderer::Camera& camera);
            private:
                Vec3f basePosition() const;
                Alignment::Type alignment() const;
                Vec2f extraOffsets(const Alignment::Type a) const;
            };
            
            class BoundsInfoMinMaxTextAnchor : public TextAnchor {
            private:
                const BBox3& m_bounds;
                BBox3::Corner m_minMax;
                const Renderer::Camera& m_camera;
            public:
                BoundsInfoMinMaxTextAnchor(const BBox3& bounds, BBox3::Corner minMax, const Renderer::Camera& camera);
            private:
                Vec3f basePosition() const;
                Alignment::Type alignment() const;
                Vec2f extraOffsets(const Alignment::Type a) const;
            };
            
            BBox3 m_bounds;
            TextRenderer<size_t> m_textRenderer;
            TextRenderer<size_t>::SimpleTextRendererFilter m_textFilter;
            TextRenderer<size_t>::PrefTextColorProvider m_textColorProvider;
            bool m_valid;
        public:
            BoundsInfoRenderer(TextureFont& font);
            
            void setBounds(const BBox3& bounds);
            void render(RenderContext& renderContext);
        private:
            void validate(RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__BoundsInfoRenderer__) */
