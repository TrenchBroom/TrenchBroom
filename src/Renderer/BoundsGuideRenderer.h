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

#ifndef __TrenchBroom__BoundsGuideRenderer__
#define __TrenchBroom__BoundsGuideRenderer__

#include "Color.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/BoundsInfoRenderer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Picker;
    }
    
    namespace Renderer {
        class RenderContext;
        class TextureFont;
        
        class BoundsGuideRenderer {
        private:
            static const FloatType SpikeLength;

            Color m_color;
            BBox3 m_bounds;

            bool m_showSizes;
            
            Vbo m_vbo;
            VertexArray m_boxArray;
            VertexArray m_spikeArray;
            VertexArray m_pointArray;
            BoundsInfoRenderer m_infoRenderer;
            
            bool m_valid;
        public:
            BoundsGuideRenderer(TextureFont& font);
            
            void setColor(const Color& color);
            void setBounds(const BBox3& bounds);
            void setShowSizes(bool showSizes);
            
            void render(RenderContext& renderContext, View::MapDocumentSPtr document);
        private:
            void validate(View::MapDocumentSPtr document);
            void validateBox();
            void validateSpikes();
            void validatePoints(View::MapDocumentSPtr document);
        };
    }
}

#endif /* defined(__TrenchBroom__BoundsGuideRenderer__) */
