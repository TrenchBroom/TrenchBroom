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

#ifndef __TrenchBroom__BrushRenderer__
#define __TrenchBroom__BrushRenderer__

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"

namespace TrenchBroom {
    namespace Model {
        class BrushEdge;
        class EditorContext;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class BrushRenderer {
        public:
            class Filter {
            public:
                virtual ~Filter();
                virtual bool operator()(const Model::Brush* brush) const = 0;
                virtual bool operator()(const Model::BrushFace* face) const = 0;
                virtual bool operator()(const Model::BrushEdge* edge) const = 0;
            };
            
            class DefaultFilter : public Filter {
            private:
                const Model::EditorContext& m_context;
            public:
                virtual ~DefaultFilter();
            protected:
                DefaultFilter(const Model::EditorContext& context);
                
                bool visible(const Model::Brush* brush) const;
                bool visible(const Model::BrushFace* face) const;
                
                bool locked(const Model::Brush* brush) const;
                bool locked(const Model::BrushFace* face) const;
                
                bool selected(const Model::Brush* brush) const;
                bool selected(const Model::BrushFace* face) const;
                bool selected(const Model::BrushEdge* edge) const;
                bool hasSelectedFaces(const Model::Brush* brush) const;
            };
        private:
            Filter* m_filter;
            Model::BrushList m_brushes;
            FaceRenderer m_opaqueFaceRenderer;
            FaceRenderer m_transparentFaceRenderer;
            EdgeRenderer m_edgeRenderer;
            bool m_valid;
            
            bool m_grayscale;
            bool m_tintFaces;
            bool m_renderOccludedEdges;
            Color m_faceColor;
            Color m_edgeColor;
            Color m_tintColor;
            Color m_occludedEdgeColor;
            float m_transparencyAlpha;
            
            bool m_showHiddenBrushes;
        public:
            template <typename FilterT>
            BrushRenderer(const FilterT& filter) :
            m_filter(new FilterT(filter)),
            m_valid(false),
            m_grayscale(false),
            m_tintFaces(false),
            m_renderOccludedEdges(false),
            m_transparencyAlpha(1.0f),
            m_showHiddenBrushes(false) {}
            
            ~BrushRenderer();

            void addBrush(Model::Brush* brush);
            void removeBrush(Model::Brush* brush);
            
            template <typename I>
            void addBrushes(I cur, I end, const size_t count) {
                m_brushes.reserve(m_brushes.size() * count);
                m_brushes.insert(m_brushes.end(), cur, end);
                invalidate();
            }
            
            template <typename I>
            void removeBrushes(I cur, I end) {
                const Model::BrushList::iterator rem = VectorUtils::removeAll(m_brushes.begin(), m_brushes.end(), cur, end);
                m_brushes.erase(rem, m_brushes.end());
                invalidate();
            }
            
            void invalidate();
            void clear();
            
            void render(RenderContext& renderContext);

            const Color& faceColor() const;
            void setFaceColor(const Color& faceColor);
            
            const Color& edgeColor() const;
            void setEdgeColor(const Color& edgeColor);

            bool grayscale() const;
            void setGrayscale(const bool grayscale);
            
            bool tintFaces() const;
            void setTintFaces(bool tintFaces);
            const Color& tintColor() const;
            void setTintColor(const Color& tintColor);
            
            bool renderOccludedEdges() const;
            void setRenderOccludedEdges(bool renderOccludedEdges);
            const Color& occludedEdgeColor() const;
            void setOccludedEdgeColor(const Color& occludedEdgeColor);
            
            float transparencyAlpha() const;
            void setTransparencyAlpha(float transparencyAlpha);
            
            bool showHiddenBrushes() const;
            void setShowHiddenBrushes(bool showHiddenBrushes);
        private:
            void renderFaces(RenderContext& renderContext);
            void renderEdges(RenderContext& renderContext);
            void validate();
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
