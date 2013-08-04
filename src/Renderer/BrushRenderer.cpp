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

#include "BrushRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/Filter.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        struct BuildBrushEdges {
            VertexSpecs::P3::Vertex::List unselectedVertices;
            VertexSpecs::P3::Vertex::List selectedVertices;
            
            inline void operator()(Model::Brush* brush) {
                if (brush->selected()) {
                    brush->addEdges(selectedVertices);
                } else if (!brush->partiallySelected()) {
                    brush->addEdges(unselectedVertices);
                } else {
                    const Model::BrushEdge::List& edges = brush->edges();
                    Model::BrushEdge::List::const_iterator it, end;
                    for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                        Model::BrushEdge* edge = *it;
                        Model::BrushFace* left = edge->left()->face();
                        Model::BrushFace* right = edge->right()->face();
                        if (left->selected() || right->selected()) {
                            selectedVertices.push_back(VertexSpecs::P3::Vertex(edge->start()->position()));
                            selectedVertices.push_back(VertexSpecs::P3::Vertex(edge->end()->position()));
                        } else {
                            unselectedVertices.push_back(VertexSpecs::P3::Vertex(edge->start()->position()));
                            unselectedVertices.push_back(VertexSpecs::P3::Vertex(edge->end()->position()));
                        }
                    }
                }
            }
        };
        
        struct BuildBrushFaceMesh {
            Model::BrushFace::Mesh unselectedMesh;
            Model::BrushFace::Mesh selectedMesh;
            
            inline void operator()(Model::Brush* brush, Model::BrushFace* face) {
                if (brush->selected() || face->selected())
                    face->addToMesh(selectedMesh);
                else
                    face->addToMesh(unselectedMesh);
            }
        };
        
        struct BuildBrushFaceMeshFilter {
        private:
            const Model::Filter& m_filter;
            bool m_unselected;
            bool m_selected;
        public:
            BuildBrushFaceMeshFilter(const Model::Filter& filter) :
            m_filter(filter),
            m_unselected(true),
            m_selected(true) {}
            
            inline void setUnselected(const bool unselected) {
                m_unselected = unselected;
            }
            
            inline void setSelected(const bool selected) {
                m_selected = selected;
            }
            
            inline bool operator()(Model::Entity* entity) const {
                if (m_unselected && !entity->selected())
                    return m_filter.visible(entity);
                if (m_selected && entity->selected())
                    return m_filter.visible(entity);
                return false;
            }
            
            inline bool operator()(Model::Brush* brush) const {
                if (m_unselected && !brush->selected())
                    return m_filter.visible(brush);
                if (m_selected && brush->selected())
                    return m_filter.visible(brush);
                return false;
            }
            
            inline bool operator()(Model::Brush* brush, Model::BrushFace* face) const {
                if (m_unselected && !face->selected())
                    return m_filter.visible(face);
                if (m_selected && face->selected())
                    return m_filter.visible(face);
                return false;
            }
        };
        
        BrushRenderer::BrushRenderer(const Model::Filter& filter) :
        m_filter(filter),
        m_unselectedValid(false),
        m_selectedValid(false) {}

        void BrushRenderer::addBrush(Model::Brush* brush) {
            m_brushes.insert(brush);
            invalidate();
        }
        
        void BrushRenderer::addBrushes(const Model::BrushList& brushes) {
            m_brushes.insert(brushes.begin(), brushes.end());
            invalidate();
        }
        
        void BrushRenderer::removeBrush(Model::Brush* brush) {
            m_brushes.erase(brush);
            invalidate();
        }
        
        void BrushRenderer::removeBrushes(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                m_brushes.erase(*it);
            invalidate();
        }

        void BrushRenderer::invalidateSelected() {
            m_selectedValid = false;
        }
        
        void BrushRenderer::invalidateUnselected() {
            m_unselectedValid = false;
        }

        void BrushRenderer::invalidate() {
            invalidateUnselected();
            invalidateSelected();
        }
        
        void BrushRenderer::clear() {
            m_brushes.clear();
            invalidate();
        }

        void BrushRenderer::render(RenderContext& context) {
            if (!m_unselectedValid || !m_selectedValid)
                validate();
            
            m_faceRenderer.render(context, false);
            m_selectedFaceRenderer.render(context, false, tintColor());
            
            glSetEdgeOffset(0.015f);
            m_edgeRenderer.setColor(edgeColor());
            m_edgeRenderer.render(context);
            glSetEdgeOffset(0.02f);
            glDisable(GL_DEPTH_TEST);
            m_selectedEdgeRenderer.setColor(occludedSelectedEdgeColor());
            m_selectedEdgeRenderer.render(context);
            glEnable(GL_DEPTH_TEST);
            m_selectedEdgeRenderer.setColor(selectedEdgeColor());
            m_selectedEdgeRenderer.render(context);
            glResetEdgeOffset();
        }
        
        void BrushRenderer::validate() {
            BuildBrushFaceMeshFilter filter(m_filter);
            filter.setUnselected(!m_unselectedValid);
            filter.setSelected(!m_selectedValid);
            
            BuildBrushFaceMesh buildFaces;
            eachFace(m_brushes, buildFaces, filter);
            
            BuildBrushEdges buildEdges;
            eachBrush(m_brushes, buildEdges, filter);
            
            if (!m_unselectedValid) {
                m_faceRenderer = FaceRenderer(buildFaces.unselectedMesh, faceColor());
                m_edgeRenderer = EdgeRenderer(buildEdges.unselectedVertices);
            }
            
            if (!m_selectedValid) {
                m_selectedFaceRenderer = FaceRenderer(buildFaces.selectedMesh, faceColor());
                m_selectedEdgeRenderer = EdgeRenderer(buildEdges.selectedVertices);
            }

            m_unselectedValid = m_selectedValid = true;
        }
        
        bool BrushRenderer::grayScale() const {
            return false;
        }
        
        const Color& BrushRenderer::faceColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::FaceColor);
        }

        const Color& BrushRenderer::tintColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::SelectedFaceColor);
        }

        const Color& BrushRenderer::edgeColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::EdgeColor);
        }

        const Color& BrushRenderer::selectedEdgeColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::SelectedEdgeColor);
        }
        
        const Color& BrushRenderer::occludedSelectedEdgeColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::OccludedSelectedEdgeColor);
        }
    }
}
