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
        class Filter;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class BrushRenderer {
        public:
            struct Filter {
                virtual ~Filter();
                virtual bool operator()(const Model::Brush* brush) const = 0;
                virtual bool operator()(const Model::BrushFace* face) const = 0;
                virtual bool operator()(const Model::BrushEdge* edge) const = 0;
            };
        private:
            struct BuildBrushEdges {
                VertexSpecs::P3::Vertex::List vertices;
                BrushRenderer::Filter& filter;
                
                BuildBrushEdges(BrushRenderer::Filter& i_filter);
                void operator()(Model::Brush* brush);
            };
            
            struct BuildBrushFaceMesh {
                Model::BrushFace::Mesh opaqueMesh;
                Model::BrushFace::Mesh transparentMesh;
                
                bool operator()(Model::BrushFace* face);
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
        public:
            template <typename FilterT>
            BrushRenderer(const FilterT& filter) :
            m_filter(new FilterT(filter)),
            m_valid(false),
            m_grayscale(false),
            m_tintFaces(false),
            m_renderOccludedEdges(false),
            m_transparencyAlpha(1.0f) {}
            
            ~BrushRenderer();

            void addBrush(Model::Brush* brush);
            void removeBrush(Model::Brush* brush);
            void setBrushes(const Model::BrushList& brushes);
            
            template <typename I>
            void addBrushes(I cur, I end) {
                while (cur != end) {
                    addBrush(*cur);
                    ++cur;
                }
            }
            
            template <typename I>
            void removeBrushes(I cur, I end) {
                while (cur != end) {
                    removeBrush(*cur);
                    ++cur;
                }
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
            void setTintFaces(const bool tintFaces);
            const Color& tintColor() const;
            void setTintColor(const Color& tintColor);
            
            bool renderOccludedEdges() const;
            void setRenderOccludedEdges(const bool renderOccludedEdges);
            const Color& occludedEdgeColor() const;
            void setOccludedEdgeColor(const Color& occludedEdgeColor);
            
            float transparencyAlpha() const;
            void setTransparencyAlpha(float transparencyAlpha);
        private:
            void renderFaces(RenderContext& renderContext);
            void renderEdges(RenderContext& renderContext);
            void validate();
        };
    }
}

#endif /* defined(__TrenchBroom__BrushRenderer__) */
