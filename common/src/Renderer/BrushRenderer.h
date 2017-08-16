/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_BrushRenderer
#define TrenchBroom_BrushRenderer

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        class Vbo;
        
        class BrushRenderer {
        public:
            class FaceAcceptor {
            public:
                virtual ~FaceAcceptor();
                virtual void accept(const Model::BrushFace* face) = 0;
            };
            
            class EdgeAcceptor {
            public:
                virtual ~EdgeAcceptor();
                virtual void accept(const Model::BrushEdge* edge) = 0;
            };
            
            class Filter {
            public:
                Filter();
                Filter(const Filter& other);
                virtual ~Filter();
                
                Filter& operator=(const Filter& other);
                
                void  provideFaces(const Model::Brush* brush, FaceAcceptor& faceAcceptor) const;
                void  provideEdges(const Model::Brush* brush, EdgeAcceptor& edgeAcceptor) const;
                
                bool transparent(const Model::Brush* brush) const;
            private:
                virtual void doProvideFaces(const Model::Brush* brush, FaceAcceptor& faceAcceptor) const = 0;
                virtual void doProvideEdges(const Model::Brush* brush, EdgeAcceptor& edgeAcceptor) const = 0;
                virtual bool doIsTransparent(const Model::Brush* brush) const = 0;
            };
            
            class DefaultFilter : public Filter {
            private:
                const Model::EditorContext& m_context;
            public:
                virtual ~DefaultFilter();
            protected:
                DefaultFilter(const Model::EditorContext& context);
                DefaultFilter(const DefaultFilter& other);
                
                bool visible(const Model::Brush* brush) const;
                bool visible(const Model::BrushFace* face) const;
                bool visible(const Model::BrushEdge* edge) const;
                
                bool editable(const Model::Brush* brush) const;
                bool editable(const Model::BrushFace* face) const;
                
                bool selected(const Model::Brush* brush) const;
                bool selected(const Model::BrushFace* face) const;
                bool selected(const Model::BrushEdge* edge) const;
                bool hasSelectedFaces(const Model::Brush* brush) const;
            private:
                DefaultFilter& operator=(const DefaultFilter& other);
            };
            
            class NoFilter : public Filter {
            private:
                bool m_transparent;
            public:
                NoFilter(bool transparent);
            private:
                void doProvideFaces(const Model::Brush* brush, FaceAcceptor& faceAcceptor) const;
                void doProvideEdges(const Model::Brush* brush, EdgeAcceptor& edgeAcceptor) const;
                bool doIsTransparent(const Model::Brush* brush) const;
            private:
                NoFilter(const NoFilter& other);
                NoFilter& operator=(const NoFilter& other);
            };
        private:
            class FilterWrapper;
            class CountVertices;
            class CollectVertices;
            class CountIndices;
            class CollectIndices;
        private:
            Filter* m_filter;
            Model::BrushList m_brushes;
            VertexArray m_vertexArray;
            FaceRenderer m_opaqueFaceRenderer;
            FaceRenderer m_transparentFaceRenderer;
            IndexedEdgeRenderer m_edgeRenderer;
            bool m_valid;
            
            Color m_faceColor;
            bool m_showEdges;
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
            m_valid(true),
            m_showEdges(false),
            m_grayscale(false),
            m_tint(false),
            m_showOccludedEdges(false),
            m_transparencyAlpha(1.0f),
            m_showHiddenBrushes(false) {}
            
            BrushRenderer(bool transparent);
            
            ~BrushRenderer();

            void addBrushes(const Model::BrushList& brushes);
            void setBrushes(const Model::BrushList& brushes);
            void clear();
            
            void invalidate();
            
            void setFaceColor(const Color& faceColor);
            void setShowEdges(bool showEdges);
            void setEdgeColor(const Color& edgeColor);
            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            void setShowOccludedEdges(bool showOccludedEdges);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);
            void setTransparencyAlpha(float transparencyAlpha);
            void setShowHiddenBrushes(bool showHiddenBrushes);
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void renderOpaqueFaces(RenderBatch& renderBatch);
            void renderTransparentFaces(RenderBatch& renderBatch);
            void renderEdges(RenderBatch& renderBatch);
            
            void validate();
            void validateVertices();
            void validateIndices();
        private:
            BrushRenderer(const BrushRenderer& other);
            BrushRenderer& operator=(const BrushRenderer& other);
        };
    }
}

#endif /* defined(TrenchBroom_BrushRenderer) */
