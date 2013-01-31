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

#include "VertexHandleManager.h"

#include "Renderer/LinesRenderer.h"
#include "Renderer/PointHandleRenderer.h"

namespace TrenchBroom {
    namespace Model {
        VertexHandleHit::VertexHandleHit(HitType::Type type, const Vec3f& hitPoint, float distance, const Vec3f& vertex) :
        Hit(type, hitPoint, distance),
        m_vertex(vertex) {
            assert(type == HitType::VertexHandleHit ||
                   type == HitType::EdgeHandleHit ||
                   type == HitType::FaceHandleHit);
        }

        bool VertexHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }

    namespace Controller {
        VertexHandleManager::VertexHandleManager() :
        m_selectedHandleRenderer(NULL),
        m_unselectedVertexHandleRenderer(NULL),
        m_unselectedEdgeHandleRenderer(NULL),
        m_unselectedFaceHandleRenderer(NULL),
        m_selectedEdgeRenderer(NULL),
        m_renderStateValid(false) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float handleRadius = prefs.getFloat(Preferences::HandleRadius);
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float maxDistance = prefs.getFloat(Preferences::MaximumHandleDistance);
            m_selectedHandleRenderer = Renderer::PointHandleRenderer::create(handleRadius, 2, scalingFactor, maxDistance);
            m_unselectedVertexHandleRenderer = Renderer::PointHandleRenderer::create(handleRadius, 2, scalingFactor, maxDistance);
            m_unselectedEdgeHandleRenderer = Renderer::PointHandleRenderer::create(handleRadius, 2, scalingFactor, maxDistance);
            m_unselectedFaceHandleRenderer = Renderer::PointHandleRenderer::create(handleRadius, 2, scalingFactor, maxDistance);
            m_selectedEdgeRenderer = new Renderer::LinesRenderer();
        }
        
        const Model::EdgeList& VertexHandleManager::edges(const Vec3f& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt == m_selectedEdgeHandles.end())
                mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt == m_unselectedEdgeHandles.end())
                return Model::EmptyEdgeList;
            return mapIt->second;
        }

        const Model::FaceList& VertexHandleManager::faces(const Vec3f& handlePosition) const {
            Model::VertexToFacesMap::const_iterator mapIt = m_selectedFaceHandles.find(handlePosition);
            if (mapIt == m_selectedFaceHandles.end())
                mapIt = m_unselectedFaceHandles.find(handlePosition);
            if (mapIt == m_unselectedFaceHandles.end())
                return Model::EmptyFaceList;
            return mapIt->second;
        }

        void VertexHandleManager::add(Model::Brush& brush) {
            const Model::VertexList& brushVertices = brush.vertices();
            Model::VertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::Vertex& vertex = **vIt;
                Model::VertexToBrushesMap::iterator mapIt = m_selectedVertexHandles.find(vertex.position);
                if (mapIt != m_selectedVertexHandles.end())
                    mapIt->second.push_back(&brush);
                else
                    m_unselectedVertexHandles[vertex.position].push_back(&brush);
            }

            const Model::EdgeList& brushEdges = brush.edges();
            Model::EdgeList::const_iterator eIt, eEnd;
            for (eIt = brushEdges.begin(), eEnd = brushEdges.end(); eIt != eEnd; ++eIt) {
                Model::Edge& edge = **eIt;
                Vec3f position = edge.center();
                Model::VertexToEdgesMap::iterator mapIt = m_selectedEdgeHandles.find(position);
                if (mapIt != m_selectedEdgeHandles.end())
                    mapIt->second.push_back(&edge);
                else
                    m_unselectedEdgeHandles[position].push_back(&edge);
            }

            const Model::FaceList& brushFaces = brush.faces();
            Model::FaceList::const_iterator fIt, fEnd;
            for (fIt = brushFaces.begin(), fEnd = brushFaces.end(); fIt != fEnd; ++fIt) {
                Model::Face& face = **fIt;
                Vec3f position = face.center();
                Model::VertexToFacesMap::iterator mapIt = m_selectedFaceHandles.find(position);
                if (mapIt != m_selectedFaceHandles.end())
                    mapIt->second.push_back(&face);
                else
                    m_unselectedFaceHandles[position].push_back(&face);
            }

            m_renderStateValid = false;
        }

        void VertexHandleManager::add(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                add(**it);
        }

        void VertexHandleManager::remove(Model::Brush& brush) {
            const Model::VertexList& brushVertices = brush.vertices();
            Model::VertexList::const_iterator vIt, vEnd;
            for (vIt = brushVertices.begin(), vEnd = brushVertices.end(); vIt != vEnd; ++vIt) {
                const Model::Vertex& vertex = **vIt;
                if (!removeHandle(vertex.position, brush, m_selectedVertexHandles))
                    removeHandle(vertex.position, brush, m_unselectedVertexHandles);
            }

            const Model::EdgeList& brushEdges = brush.edges();
            Model::EdgeList::const_iterator eIt, eEnd;
            for (eIt = brushEdges.begin(), eEnd = brushEdges.end(); eIt != eEnd; ++eIt) {
                Model::Edge& edge = **eIt;
                Vec3f position = edge.center();
                if (!removeHandle(position, edge, m_selectedEdgeHandles))
                    removeHandle(position, edge, m_unselectedEdgeHandles);
            }

            const Model::FaceList& brushFaces = brush.faces();
            Model::FaceList::const_iterator fIt, fEnd;
            for (fIt = brushFaces.begin(), fEnd = brushFaces.end(); fIt != fEnd; ++fIt) {
                Model::Face& face = **fIt;
                Vec3f position = face.center();
                if (!removeHandle(position, face, m_selectedFaceHandles))
                    removeHandle(position, face, m_unselectedFaceHandles);
            }

            m_renderStateValid = false;
        }

        void VertexHandleManager::remove(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                remove(**it);
        }

        void VertexHandleManager::clear() {
            m_unselectedVertexHandles.clear();
            m_selectedVertexHandles.clear();
            m_unselectedEdgeHandles.clear();
            m_selectedEdgeHandles.clear();
            m_unselectedFaceHandles.clear();
            m_selectedFaceHandles.clear();
            m_renderStateValid = false;
        }

        void VertexHandleManager::selectVertexHandle(const Vec3f& position) {
            if (moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::deselectVertexHandle(const Vec3f& position) {
            if (moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::selectVertexHandles(const Vec3f::Set& positions) {
            Vec3f::Set::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                selectVertexHandle(*it);
        }

        void VertexHandleManager::deselectVertexHandles() {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                const Model::BrushList& selectedBrushes = vIt->second;
                Model::BrushList& unselectedBrushes = m_unselectedVertexHandles[position];
                unselectedBrushes.insert(unselectedBrushes.begin(), selectedBrushes.begin(), selectedBrushes.end());
            }
            m_selectedVertexHandles.clear();
            m_renderStateValid = false;
        }

        void VertexHandleManager::selectEdgeHandle(const Vec3f& position) {
            if (moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::deselectEdgeHandle(const Vec3f& position) {
            if (moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::selectEdgeHandles(const Model::EdgeList& edges) {
            Model::EdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::Edge& edge = **it;
                selectEdgeHandle(edge.center());
            }
        }

        void VertexHandleManager::deselectEdgeHandles() {
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3f& position = eIt->first;
                const Model::EdgeList& selectedEdges = eIt->second;
                Model::EdgeList& unselectedEdges = m_unselectedEdgeHandles[position];
                unselectedEdges.insert(unselectedEdges.begin(), selectedEdges.begin(), selectedEdges.end());
            }
            m_selectedEdgeHandles.clear();
            m_renderStateValid = false;
        }

        void VertexHandleManager::selectFaceHandle(const Vec3f& position) {
            if (moveHandle(position, m_unselectedFaceHandles, m_selectedFaceHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::deselectFaceHandle(const Vec3f& position) {
            if (moveHandle(position, m_selectedFaceHandles, m_unselectedFaceHandles))
                m_renderStateValid = false;
        }

        void VertexHandleManager::selectFaceHandles(const Model::FaceList& faces) {
            Model::FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::Face& face = **it;
                selectFaceHandle(face.center());
            }
        }

        void VertexHandleManager::deselectFaceHandles() {
            Model::VertexToFacesMap::const_iterator fIt, fEnd;
            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                const Model::FaceList& selectedFaces = fIt->second;
                Model::FaceList& unselectedFaces = m_unselectedFaceHandles[position];
                unselectedFaces.insert(unselectedFaces.begin(), selectedFaces.begin(), selectedFaces.end());
            }
            m_selectedFaceHandles.clear();
            m_renderStateValid = false;
        }

        void VertexHandleManager::deselectAll() {
            deselectVertexHandles();
            deselectEdgeHandles();
            deselectFaceHandles();
        }

        void VertexHandleManager::pick(const Ray& ray, Model::PickResult& pickResult, bool splitMode) const {
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            Model::VertexToFacesMap::const_iterator fIt, fEnd;

            if ((m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) || splitMode) {
                for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::VertexHandleHit);
                    if (hit != NULL)
                        pickResult.add(hit);
                }
            }

            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::VertexHandleHit);
                if (hit != NULL)
                    pickResult.add(hit);
            }

            if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3f& position = eIt->first;
                    Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::EdgeHandleHit);
                    if (hit != NULL)
                        pickResult.add(hit);
                }
            }

            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3f& position = eIt->first;
                Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::EdgeHandleHit);
                if (hit != NULL)
                    pickResult.add(hit);
            }

            if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                    const Vec3f& position = fIt->first;
                    Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::FaceHandleHit);
                    if (hit != NULL)
                        pickResult.add(hit);
                }
            }

            for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                const Vec3f& position = fIt->first;
                Model::VertexHandleHit* hit = pickHandle(ray, position, Model::HitType::FaceHandleHit);
                if (hit != NULL)
                    pickResult.add(hit);
            }
        }

        void VertexHandleManager::render(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, bool splitMode) {
            if (!m_renderStateValid) {
                Model::VertexToBrushesMap::const_iterator vIt, vEnd;
                Model::VertexToEdgesMap::const_iterator eIt, eEnd;
                Model::VertexToFacesMap::const_iterator fIt, fEnd;

                m_unselectedVertexHandleRenderer->clear();
                m_unselectedEdgeHandleRenderer->clear();
                m_unselectedFaceHandleRenderer->clear();
                m_selectedHandleRenderer->clear();
                m_selectedEdgeRenderer->clear();

                unsigned int vertexCount = 0;
                for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Model::EdgeList& edges = eIt->second;
                    vertexCount += static_cast<unsigned int>(2 * edges.size());
                }
                
                for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                    const Model::FaceList& faces = fIt->second;
                    Model::FaceList::const_iterator faceIt, faceEnd;
                    for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                        const Model::Face& face = **faceIt;
                        vertexCount += static_cast<unsigned int>(2 * face.edges().size());
                    }
                }
                
                if ((m_selectedEdgeHandles.empty() && m_selectedFaceHandles.empty()) || splitMode) {
                    for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                        const Vec3f& position = vIt->first;
                        m_unselectedVertexHandleRenderer->add(position);
                    }
                }

                for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    m_selectedHandleRenderer->add(position);
                }

                if (m_selectedVertexHandles.empty() && m_selectedFaceHandles.empty() && !splitMode) {
                    for (eIt = m_unselectedEdgeHandles.begin(), eEnd = m_unselectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                        const Vec3f& position = eIt->first;
                        m_unselectedEdgeHandleRenderer->add(position);
                    }
                }

                for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3f& position = eIt->first;
                    m_selectedHandleRenderer->add(position);
                    
                    const Model::EdgeList& edges = eIt->second;
                    Model::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::Edge& edge = **edgeIt;
                        m_selectedEdgeRenderer->add(edge.start->position, edge.end->position);
                    }
                }

                if (m_selectedVertexHandles.empty() && m_selectedEdgeHandles.empty() && !splitMode) {
                    for (fIt = m_unselectedFaceHandles.begin(), fEnd = m_unselectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                        const Vec3f& position = fIt->first;
                        m_unselectedFaceHandleRenderer->add(position);
                    }
                }

                for (fIt = m_selectedFaceHandles.begin(), fEnd = m_selectedFaceHandles.end(); fIt != fEnd; ++fIt) {
                    const Vec3f& position = fIt->first;
                    m_selectedHandleRenderer->add(position);
                    
                    const Model::FaceList& faces = fIt->second;
                    Model::FaceList::const_iterator faceIt, faceEnd;
                    for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                        const Model::Face& face = **faceIt;
                        const Model::EdgeList& edges = face.edges();
                        Model::EdgeList::const_iterator edgeIt, edgeEnd;
                        for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                            const Model::Edge& edge = **edgeIt;
                            m_selectedEdgeRenderer->add(edge.start->position, edge.end->position);
                        }
                    }
                }

                m_renderStateValid = true;
            }

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            m_selectedEdgeRenderer->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(1.0f, 1.0f, 1.0f, 0.5f));
            m_selectedEdgeRenderer->render(vbo, renderContext);
            
            m_unselectedVertexHandleRenderer->setColor(prefs.getColor(Preferences::VertexHandleColor));
            m_unselectedEdgeHandleRenderer->setColor(prefs.getColor(Preferences::EdgeHandleColor));
            m_unselectedFaceHandleRenderer->setColor(prefs.getColor(Preferences::FaceHandleColor));
            if (splitMode)
                m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::SelectedSplitHandleColor));
            else
                m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::SelectedVertexHandleColor));

            m_unselectedVertexHandleRenderer->render(vbo, renderContext);
            m_unselectedEdgeHandleRenderer->render(vbo, renderContext);
            m_unselectedFaceHandleRenderer->render(vbo, renderContext);
            m_selectedHandleRenderer->render(vbo, renderContext);

            m_unselectedVertexHandleRenderer->setColor(prefs.getColor(Preferences::OccludedVertexHandleColor));
            m_unselectedEdgeHandleRenderer->setColor(prefs.getColor(Preferences::OccludedEdgeHandleColor));
            m_unselectedFaceHandleRenderer->setColor(prefs.getColor(Preferences::OccludedFaceHandleColor));
            if (splitMode)
                m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::OccludedSelectedSplitHandleColor));
            else
                m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::OccludedSelectedVertexHandleColor));

            glDisable(GL_DEPTH_TEST);
            m_unselectedVertexHandleRenderer->render(vbo, renderContext);
            m_unselectedEdgeHandleRenderer->render(vbo, renderContext);
            m_unselectedFaceHandleRenderer->render(vbo, renderContext);
            m_selectedHandleRenderer->render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        void VertexHandleManager::freeRenderResources() {
            delete m_unselectedVertexHandleRenderer;
            m_unselectedVertexHandleRenderer = NULL;
            delete m_unselectedEdgeHandleRenderer;
            m_unselectedEdgeHandleRenderer = NULL;
            delete m_unselectedFaceHandleRenderer;
            m_unselectedFaceHandleRenderer = NULL;
            delete m_selectedHandleRenderer;
            m_selectedHandleRenderer = NULL;
            delete m_selectedEdgeRenderer;
            m_selectedEdgeRenderer = NULL;
        }
    }
}
