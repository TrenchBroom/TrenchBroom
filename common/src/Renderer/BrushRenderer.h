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
        class RenderBatch;
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
            
            Color m_faceColor;
            Color m_edgeColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            bool m_showOccludedEdges;
            Color m_occludedEdgeColor;
            float m_transparencyAlpha;
            
            bool m_showHiddenBrushes;
        public:
            template <typename FilterT>
            BrushRenderer(const FilterT& filter) :
            m_filter(new FilterT(filter)),
            m_valid(false),
            m_grayscale(false),
            m_tint(false),
            m_showOccludedEdges(false),
            m_transparencyAlpha(1.0f),
            m_showHiddenBrushes(false) {}
            
            ~BrushRenderer();

            void addBrush(Model::Brush* brush);
            void removeBrush(Model::Brush* brush);
            void updateBrush(Model::Brush* brush);
            
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
            
            template <typename I>
            void updateBBrushes(I cur, I end) {
                invalidate();
            }
            
            void invalidate();
            void clear();
            
            void setFaceColor(const Color& faceColor);
            void setEdgeColor(const Color& edgeColor);
            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            void setShowOccludedEdges(bool renderOccludedEdges);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);
            void setTransparencyAlpha(float transparencyAlpha);
            void setShowHiddenBrushes(bool showHiddenBrushes);
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void renderFaces(RenderBatch& renderBatch);
            void renderEdges(RenderBatch& renderBatch);
            void validate();
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
