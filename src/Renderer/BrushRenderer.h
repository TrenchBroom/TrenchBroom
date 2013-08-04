/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__BrushRenderer__
#define __TrenchBroom__BrushRenderer__

#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"

namespace TrenchBroom {
    namespace Model {
        class Filter;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class BrushRenderer {
        private:
            const Model::Filter& m_filter;
            bool m_unselectedValid;
            bool m_selectedValid;
            Model::BrushSet m_brushes;
            FaceRenderer m_faceRenderer;
            FaceRenderer m_selectedFaceRenderer;
            EdgeRenderer m_edgeRenderer;
            EdgeRenderer m_selectedEdgeRenderer;
        public:
            BrushRenderer(const Model::Filter& filter);
            
            void addBrush(Model::Brush* brush);
            void addBrushes(const Model::BrushList& brushes);
            void removeBrush(Model::Brush* brush);
            void removeBrushes(const Model::BrushList& brushes);
            void invalidateSelected();
            void invalidateUnselected();
            void invalidate();
            void clear();
            
            void render(RenderContext& context);
        private:
            void validate();
            
            bool grayScale() const;
            const Color& faceColor() const;
            const Color& tintColor() const;
            const Color& edgeColor() const;
            const Color& selectedEdgeColor() const;
            const Color& occludedSelectedEdgeColor() const;
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
