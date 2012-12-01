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

#ifndef __TrenchBroom__MoveVertexTool__
#define __TrenchBroom__MoveVertexTool__

#include "Controller/Tool.h"
#include "Controller/MoveHandle.h"
#include "Renderer/RenderTypes.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>
#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;

        namespace HitType {
            static const Type VertexHandleHit    = 1 << 6;
        }
        
        class VertexHandleHit : public Hit {
        private:
            Vec3f m_vertex;
        public:
            VertexHandleHit(const Vec3f& hitPoint, float distance, const Vec3f& vertex);
            bool pickable(Filter& filter) const;
            
            inline const Vec3f& vertex() const {
                return m_vertex;
            }
        };
    }

    namespace Renderer {
        class ManyCubesInstancedFigure;
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class VertexSelection {
        public:
            typedef std::map<Vec3f, Model::BrushList, Vec3f::LexicographicOrder> VertexBrushMap;
        private:
            VertexBrushMap m_vertices;
            VertexBrushMap m_selectedVertices;
            
            inline bool removeVertex(const Vec3f& vertex, Model::Brush& brush, VertexBrushMap& map) {
                VertexBrushMap::iterator mapIt = map.find(vertex);
                if (mapIt == map.end())
                    return false;
                
                Model::BrushList& vertexBrushes = mapIt->second;
                Model::BrushList::iterator brushIt = std::find(vertexBrushes.begin(), vertexBrushes.end(), &brush);
                if (brushIt == vertexBrushes.end())
                    return false;
                
                vertexBrushes.erase(brushIt);
                if (vertexBrushes.empty())
                    map.erase(mapIt);
                return true;
            }
            
            inline bool moveVertex(const Vec3f& vertex, VertexBrushMap& from, VertexBrushMap& to) {
                VertexBrushMap::iterator mapIt = from.find(vertex);
                if (mapIt == from.end())
                    return false;
                
                Model::BrushList& fromBrushes = mapIt->second;
                Model::BrushList& toBrushes = to[vertex];
                toBrushes.insert(toBrushes.end(), fromBrushes.begin(), fromBrushes.end());
                
                from.erase(mapIt);
                return true;
            }
        public:
            inline const VertexBrushMap& vertices() const {
                return m_vertices;
            }
            
            inline const VertexBrushMap& selectedVertices() const {
                return m_selectedVertices;
            }
            
            inline void addVertices(Model::Brush& brush) {
                const Model::VertexList& brushVertices = brush.vertices();
                Model::VertexList::const_iterator it, end;
                for (it = brushVertices.begin(), end = brushVertices.end(); it != end; ++it) {
                    const Model::Vertex& vertex = **it;
                    VertexBrushMap::iterator mapIt = m_selectedVertices.find(vertex.position);
                    if (mapIt != m_selectedVertices.end())
                        mapIt->second.push_back(&brush);
                    else
                        m_vertices[vertex.position].push_back(&brush);
                }
            }
            
            inline void addVertices(const Model::BrushList& brushes) {
                Model::BrushList::const_iterator it, end;
                for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                    addVertices(**it);
            }
            
            inline void removeVertices(Model::Brush& brush) {
                const Model::VertexList& brushVertices = brush.vertices();
                Model::VertexList::const_iterator it, end;
                for (it = brushVertices.begin(), end = brushVertices.end(); it != end; ++it) {
                    const Model::Vertex& vertex = **it;
                    if (!removeVertex(vertex.position, brush, m_selectedVertices))
                        removeVertex(vertex.position, brush, m_vertices);
                }
            }
            
            inline void removeVertices(const Model::BrushList& brushes) {
                Model::BrushList::const_iterator it, end;
                for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                    removeVertices(**it);
            }
            
            inline bool selected(const Vec3f& vertex) {
                return m_selectedVertices.find(vertex) != m_selectedVertices.end();
            }
            
            inline bool selectVertex(const Vec3f& vertex) {
                return moveVertex(vertex, m_vertices, m_selectedVertices);
            }
            
            inline bool deselectVertex(const Vec3f& vertex) {
                return moveVertex(vertex, m_selectedVertices, m_vertices);
            }
            
            inline void deselectAll() {
                VertexBrushMap::const_iterator it, end;
                for (it = m_selectedVertices.begin(), end = m_selectedVertices.end(); it != end; ++it) {
                    const Vec3f& vertex = it->first;
                    const Model::BrushList& selectedBrushes = it->second;
                    Model::BrushList& unselectedBrushes = m_vertices[vertex];
                    unselectedBrushes.insert(unselectedBrushes.begin(), selectedBrushes.begin(), selectedBrushes.end());
                }
                m_selectedVertices.clear();
            }
            
            inline void clear() {
                m_vertices.clear();
                m_selectedVertices.clear();
            }
        };
        
        class MoveVertexTool : public PlaneDragTool {
        protected:
            VertexSelection m_selection;
            MoveHandle m_moveHandle;
            float m_vertexHandleSize;
            Vec3f m_totalDelta;
            MoveHandle::RestrictToAxis m_restrictToAxis;

            Renderer::ManyCubesInstancedFigure* m_vertexFigure;
            Renderer::ManyCubesInstancedFigure* m_selectedVertexFigure;
            bool m_vertexFigureValid;

            void updateMoveHandle(InputState& inputState);
            
            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            bool handleUpdateState(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            bool handleMouseUp(InputState& inputState);
            void handleMouseMove(InputState& inputState);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            void handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) ;
            void handleEndPlaneDrag(InputState& inputState);

            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
            void handleCameraChange(InputState& inputState);
        public:
            MoveVertexTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVertexTool__) */
