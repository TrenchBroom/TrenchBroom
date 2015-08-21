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

#ifndef __TrenchBroom__CreateBrushTool__
#define __TrenchBroom__CreateBrushTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Polyhedron.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class BrushRenderer;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class CreateBrushTool : public Tool {
        private:
            MapDocumentWPtr m_document;
            Model::Brush* m_brush;
            Renderer::BrushRenderer* m_brushRenderer;
        public:
            CreateBrushTool(MapDocumentWPtr document);
            ~CreateBrushTool();
            
            void updateBrush(const BBox3& bounds);
            void updateBrush(const Polyhedron3& polyhedron);
        public:
            void createBrush();
            
            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void renderBrush(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool doActivate();
            String doGetIconName() const;
        };
    }
}

#endif /* defined(__TrenchBroom__CreateBrushTool__) */
