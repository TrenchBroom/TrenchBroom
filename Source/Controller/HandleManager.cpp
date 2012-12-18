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

#include "HandleManager.h"

#include "Renderer/PointHandleRenderer.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        HandleManager::HandleManager() :
        m_renderStateValid(false) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float handleRadius = prefs.getFloat(Preferences::VertexHandleRadius);
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float maxDistance = prefs.getFloat(Preferences::MaximumHandleDistance);
            m_selectedHandleRenderer = Renderer::PointHandleRendererPtr(new Renderer::PointHandleRenderer(handleRadius, 2, scalingFactor, maxDistance));
            m_unselectedHandleRenderer = Renderer::PointHandleRendererPtr(new Renderer::PointHandleRenderer(handleRadius, 2, scalingFactor, maxDistance));
        }
        
        const Model::EdgeList& HandleManager::edges(const Vec3f& handlePosition) const {
            Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
            if (mapIt == m_selectedEdgeHandles.end())
                mapIt = m_unselectedEdgeHandles.find(handlePosition);
            if (mapIt == m_unselectedEdgeHandles.end())
                return Model::EmptyEdgeList;
            return mapIt->second;
        }
        
        void HandleManager::add(Model::Brush& brush) {
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
            m_renderStateValid = false;
        }
        
        void HandleManager::add(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                add(**it);
        }
        
        void HandleManager::remove(Model::Brush& brush) {
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
            m_renderStateValid = false;
        }
        
        void HandleManager::remove(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                remove(**it);
        }
        
        void HandleManager::clear() {
            m_unselectedVertexHandles.clear();
            m_selectedVertexHandles.clear();
            m_unselectedEdgeHandles.clear();
            m_selectedEdgeHandles.clear();
            clearSavedSelection();
            m_renderStateValid = false;
        }

        bool HandleManager::selectVertexHandle(const Vec3f& position) {
            return moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles);
            m_renderStateValid = false;
        }
        
        bool HandleManager::deselectVertexHandle(const Vec3f& position) {
            return moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles);
            m_renderStateValid = false;
        }
        
        bool HandleManager::selectVertexHandles(const Vec3f::Set& positions) {
            Vec3f::Set::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                selectVertexHandle(*it);
            m_renderStateValid = false;
            return true;
        }
        
        void HandleManager::deselectVertexHandles() {
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
        
        bool HandleManager::selectEdgeHandle(const Vec3f& position) {
            m_renderStateValid = false;
            return moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles);
        }

        bool HandleManager::deselectEdgeHandle(const Vec3f& position) {
            m_renderStateValid = false;
            return moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles);
        }
        
        void HandleManager::deselectEdgeHandles() {
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
        
        void HandleManager::deselectAll() {
            deselectVertexHandles();
            deselectEdgeHandles();
            m_renderStateValid = false;
        }
        
        void HandleManager::saveSelection() {
            clearSavedSelection();
            
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                m_savedVertexSelection.push_back(position);
            }
            
            Model::VertexToEdgesMap::const_iterator eIt, eEnd;
            for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                const Vec3f& position = eIt->first;
                m_savedEdgeSelection.push_back(position);
            }
        }
        
        void HandleManager::clearSavedSelection() {
            m_savedVertexSelection.clear();
            m_savedEdgeSelection.clear();
        }
        
        void HandleManager::restoreSavedSelection() {
            deselectAll();
            
            Vec3f::List::const_iterator pIt, pEnd;
            for (pIt = m_savedVertexSelection.begin(), pEnd = m_savedVertexSelection.end(); pIt != pEnd; ++pIt)
                selectVertexHandle(*pIt);
            for (pIt = m_savedEdgeSelection.begin(), pEnd = m_savedEdgeSelection.end(); pIt != pEnd; ++pIt)
                selectEdgeHandle(*pIt);
            clearSavedSelection();
            m_renderStateValid = false;
        }
        
        void HandleManager::pick(const Ray& ray, Model::PickResult& pickResult) const {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float handleRadius = prefs.getFloat(Preferences::VertexHandleRadius);
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float maxDistance = prefs.getFloat(Preferences::MaximumHandleDistance);

            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                float distanceToHandle = (position - ray.origin).length();
                if (distanceToHandle <= maxDistance) {
                    float scaledRadius = handleRadius * scalingFactor * distanceToHandle;
                    float distanceToHit = ray.intersectWithSphere(position, scaledRadius);
                    
                    if (!Math::isnan(distanceToHit)) {
                        Vec3f hitPoint = ray.pointAtDistance(distanceToHit);
                        pickResult.add(new Model::VertexHandleHit(Model::HitType::VertexHandleHit, hitPoint, distanceToHit, position));
                    }
                }
            }
            
            for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                const Vec3f& position = vIt->first;
                float distanceToHandle = (position - ray.origin).length();
                if (distanceToHandle <= maxDistance) {
                    float scaledRadius = handleRadius * scalingFactor * distanceToHandle;
                    float distanceToHit = ray.intersectWithSphere(position, scaledRadius);
                    
                    if (!Math::isnan(distanceToHit)) {
                        Vec3f hitPoint = ray.pointAtDistance(distanceToHit);
                        pickResult.add(new Model::VertexHandleHit(Model::HitType::VertexHandleHit, hitPoint, distanceToHit, position));
                    }
                }
            }
        }
        
        void HandleManager::render(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (!m_renderStateValid) {
                m_unselectedHandleRenderer->clear();
                m_selectedHandleRenderer->clear();
                
                Model::VertexToBrushesMap::const_iterator vIt, vEnd;
                for (vIt = m_unselectedVertexHandles.begin(), vEnd = m_unselectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    m_unselectedHandleRenderer->add(position);
                }
                
                for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    m_selectedHandleRenderer->add(position);
                }
                
                m_renderStateValid = true;
            }
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_unselectedHandleRenderer->setColor(prefs.getColor(Preferences::VertexHandleColor));
            m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::SelectedVertexHandleColor));
            
            m_unselectedHandleRenderer->render(vbo, renderContext);
            m_selectedHandleRenderer->render(vbo, renderContext);
            
            m_unselectedHandleRenderer->setColor(prefs.getColor(Preferences::OccludedVertexHandleColor));
            m_selectedHandleRenderer->setColor(prefs.getColor(Preferences::OccludedSelectedVertexHandleColor));
            
            glDisable(GL_DEPTH_TEST);
            m_unselectedHandleRenderer->render(vbo, renderContext);
            m_selectedHandleRenderer->render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }
    }
}