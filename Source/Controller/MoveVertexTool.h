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
            Brush& m_brush;
            Vec3f m_vertex;
        public:
            VertexHandleHit(const Vec3f& hitPoint, float distance, Brush& brush, const Vec3f& vertex);
            bool pickable(Filter& filter) const;
            
            inline Brush& brush() const {
                return m_brush;
            }
            
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
        private:
            typedef std::map<Model::Brush*, Vec3f::Set> SelectionMap;
            SelectionMap m_selection;
        public:
            inline const Vec3f::Set& vertices(Model::Brush* brush) const {
                SelectionMap::const_iterator it = m_selection.find(brush);
                assert(it != m_selection.end());
                return it->second;
            }
            
            inline void addVertex(Model::Brush* brush, const Vec3f& vertex) {
                m_selection[brush].insert(vertex);
            }
            
            inline bool containsBrush(Model::Brush* brush) const {
                SelectionMap::const_iterator mapIt = m_selection.find(brush);
                return mapIt != m_selection.end();
            }
            
            inline bool containsVertex(Model::Brush* brush, const Vec3f& vertex) const {
                SelectionMap::const_iterator mapIt = m_selection.find(brush);
                if (mapIt == m_selection.end())
                    return false;
                const Vec3f::Set& vertices = mapIt->second;
                return vertices.count(vertex) > 0;
            }
            
            inline void removeVertex(Model::Brush* brush, const Vec3f& vertex) {
                SelectionMap::iterator it = m_selection.find(brush);
                assert(it != m_selection.end());
                Vec3f::Set& vertices = it->second;
                size_t count = vertices.erase(vertex);
                assert(count > 0);
            }
            
            inline void replaceVertices(Model::Brush* brush, const Vec3f::Set& vertices) {
                m_selection[brush] = vertices;
            }
            
            inline void removeVertices(Model::Brush* brush) {
                SelectionMap::iterator it = m_selection.find(brush);
                assert(it != m_selection.end());
                m_selection.erase(it);
            }
            
            inline void clear() {
                m_selection.clear();
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

            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            bool handleUpdateState(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            bool handleMouseUp(InputState& inputState);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            void handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) ;
            void handleEndPlaneDrag(InputState& inputState);

            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
        public:
            MoveVertexTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVertexTool__) */
