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

#ifndef __TrenchBroom__MoveVerticesTool__
#define __TrenchBroom__MoveVerticesTool__

#include "Controller/Tool.h"
#include "Controller/MoveHandle.h"
#include "Model/BrushGeometryTypes.h"
#include "Renderer/RenderTypes.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;

        namespace HitType {
            static const Type VertexHandleHit    = 1 << 6;
            static const Type EdgeHandleHit      = 1 << 7;
        }
        
        class VertexHandleHit : public Hit {
        private:
            Vec3f m_vertex;
        public:
            VertexHandleHit(HitType::Type type, const Vec3f& hitPoint, float distance, const Vec3f& vertex);
            bool pickable(Filter& filter) const;
            
            inline const Vec3f& vertex() const {
                return m_vertex;
            }
        };
    }

    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class HandleManager {
        private:
            Model::VertexToBrushesMap m_unselectedVertexHandles;
            Model::VertexToBrushesMap m_selectedVertexHandles;
            Model::VertexToEdgesMap m_unselectedEdgeHandles;
            Model::VertexToEdgesMap m_selectedEdgeHandles;
            Vec3f::List m_savedVertexSelection;
            Vec3f::List m_savedEdgeSelection;
            
            template <typename Element>
            inline bool removeHandle(const Vec3f& position, Element& element, std::map<Vec3f, std::vector<Element*>, Vec3f::LexicographicOrder >& map) {
                typedef std::vector<Element*> List;
                typedef std::map<Vec3f, List, Vec3f::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = map.find(position);
                if (mapIt == map.end())
                    return false;
                
                List& elements = mapIt->second;
                typename List::iterator listIt = std::find(elements.begin(), elements.end(), &element);
                if (listIt == elements.end())
                    return false;
                
                elements.erase(listIt);
                if (elements.empty())
                    map.erase(mapIt);
                return true;
            }
            
            template <typename Element>
            inline bool moveHandle(const Vec3f& position, std::map<Vec3f, std::vector<Element*>, Vec3f::LexicographicOrder >& from, std::map<Vec3f, std::vector<Element*>, Vec3f::LexicographicOrder >& to) {
                typedef std::vector<Element*> List;
                typedef std::map<Vec3f, List, Vec3f::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = from.find(position);
                if (mapIt == from.end())
                    return false;
                
                List& fromElements = mapIt->second;
                List& toElements = to[position];
                toElements.insert(toElements.end(), fromElements.begin(), fromElements.end());
                
                from.erase(mapIt);
                return true;
            }
            
        public:
            inline const Model::VertexToBrushesMap& unselectedVertexHandles() const {
                return m_unselectedVertexHandles;
            }
            
            inline const Model::VertexToBrushesMap& selectedVertexHandles() const {
                return m_selectedVertexHandles;
            }
            
            inline const Model::VertexToEdgesMap& unselectedEdgeHandles() const {
                return m_unselectedEdgeHandles;
            }
            
            inline const Model::VertexToEdgesMap& selectedEdgeHandles() const {
                return m_selectedEdgeHandles;
            }
            
            inline const Model::EdgeList& edges(const Vec3f& handlePosition) const {
                Model::VertexToEdgesMap::const_iterator mapIt = m_selectedEdgeHandles.find(handlePosition);
                if (mapIt == m_selectedEdgeHandles.end())
                    mapIt = m_unselectedEdgeHandles.find(handlePosition);
                if (mapIt == m_unselectedEdgeHandles.end())
                    return Model::EmptyEdgeList;
                return mapIt->second;
            }
            
            inline void add(Model::Brush& brush) {
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
            }
            
            inline void add(const Model::BrushList& brushes) {
                Model::BrushList::const_iterator it, end;
                for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                    add(**it);
            }
            
            inline void remove(Model::Brush& brush) {
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
            }
            
            inline void remove(const Model::BrushList& brushes) {
                Model::BrushList::const_iterator it, end;
                for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                    remove(**it);
            }
            
            inline bool vertexHandleSelected(const Vec3f& position) {
                return m_selectedVertexHandles.find(position) != m_selectedVertexHandles.end();
            }
            
            inline bool selectVertexHandle(const Vec3f& position) {
                return moveHandle(position, m_unselectedVertexHandles, m_selectedVertexHandles);
            }
            
            inline bool selectVertexHandles(const Vec3f::Set& positions) {
                Vec3f::Set::const_iterator it, end;
                for (it = positions.begin(), end = positions.end(); it != end; ++it)
                    selectVertexHandle(*it);
                return true;
            }
            
            inline bool deselectVertexHandle(const Vec3f& position) {
                return moveHandle(position, m_selectedVertexHandles, m_unselectedVertexHandles);
            }
            
            inline void deselectVertexHandles() {
                Model::VertexToBrushesMap::const_iterator vIt, vEnd;
                for (vIt = m_selectedVertexHandles.begin(), vEnd = m_selectedVertexHandles.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& position = vIt->first;
                    const Model::BrushList& selectedBrushes = vIt->second;
                    Model::BrushList& unselectedBrushes = m_unselectedVertexHandles[position];
                    unselectedBrushes.insert(unselectedBrushes.begin(), selectedBrushes.begin(), selectedBrushes.end());
                }
                m_selectedVertexHandles.clear();
            }

            inline bool edgeHandleSelected(const Vec3f& position) {
                return m_selectedEdgeHandles.find(position) != m_selectedEdgeHandles.end();
            }
            
            inline bool selectEdgeHandle(const Vec3f& position) {
                return moveHandle(position, m_unselectedEdgeHandles, m_selectedEdgeHandles);
            }
            
            inline bool deselectEdgeHandle(const Vec3f& position) {
                return moveHandle(position, m_selectedEdgeHandles, m_unselectedEdgeHandles);
            }
            
            inline void deselectEdgeHandles() {
                Model::VertexToEdgesMap::const_iterator eIt, eEnd;
                for (eIt = m_selectedEdgeHandles.begin(), eEnd = m_selectedEdgeHandles.end(); eIt != eEnd; ++eIt) {
                    const Vec3f& position = eIt->first;
                    const Model::EdgeList& selectedEdges = eIt->second;
                    Model::EdgeList& unselectedEdges = m_unselectedEdgeHandles[position];
                    unselectedEdges.insert(unselectedEdges.begin(), selectedEdges.begin(), selectedEdges.end());
                }
                m_selectedEdgeHandles.clear();
            }

            inline void deselectAll() {
                deselectVertexHandles();
                deselectEdgeHandles();
            }
            
            inline void saveSelection() {
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
            
            inline void clearSavedSelection() {
                m_savedVertexSelection.clear();
                m_savedEdgeSelection.clear();
            }
            
            inline void restoreSavedSelection() {
                deselectAll();
                
                Vec3f::List::const_iterator pIt, pEnd;
                for (pIt = m_savedVertexSelection.begin(), pEnd = m_savedVertexSelection.end(); pIt != pEnd; ++pIt)
                    selectVertexHandle(*pIt);
                for (pIt = m_savedEdgeSelection.begin(), pEnd = m_savedEdgeSelection.end(); pIt != pEnd; ++pIt)
                    selectEdgeHandle(*pIt);
                clearSavedSelection();
            }
            
            inline void clear() {
                m_unselectedVertexHandles.clear();
                m_selectedVertexHandles.clear();
                m_unselectedEdgeHandles.clear();
                m_selectedEdgeHandles.clear();
                clearSavedSelection();
            }
        };
        
        class MoveVerticesTool : public PlaneDragTool {
        protected:
            HandleManager m_handleManager;
            MoveHandle m_moveHandle;
            float m_vertexHandleSize;
            MoveHandle::RestrictToAxis m_restrictToAxis;

            void updateMoveHandle(InputState& inputState);
            
            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            bool handleUpdateState(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            void handleModifierKeyChange(InputState& inputState);
            bool handleMouseUp(InputState& inputState);
            void handleMouseMove(InputState& inputState);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            void handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) ;
            void handleEndPlaneDrag(InputState& inputState);

            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
            void handleCameraChange(InputState& inputState);
        public:
            MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVerticesTool__) */
