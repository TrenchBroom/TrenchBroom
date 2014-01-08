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

#ifndef __TrenchBroom__VertexHandleManager__
#define __TrenchBroom__VertexHandleManager__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/Vbo.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class VertexHandleManager {
        public:
            static const Model::Hit::HitType VertexHandleHit;
            static const Model::Hit::HitType EdgeHandleHit;
            static const Model::Hit::HitType FaceHandleHit;
        private:
            Renderer::Vbo m_vbo;
            
            Model::VertexToBrushesMap m_unselectedVertexHandles;
            Model::VertexToBrushesMap m_selectedVertexHandles;
            Model::VertexToEdgesMap m_unselectedEdgeHandles;
            Model::VertexToEdgesMap m_selectedEdgeHandles;
            Model::VertexToFacesMap m_unselectedFaceHandles;
            Model::VertexToFacesMap m_selectedFaceHandles;
            
            size_t m_totalVertexCount;
            size_t m_selectedVertexCount;
            size_t m_totalEdgeCount;
            size_t m_selectedEdgeCount;
            size_t m_totalFaceCount;
            size_t m_selectedFaceCount;
            
            Vec3f::List m_unselectedVertexHandlePositions;
            Vec3f::List m_unselectedEdgeHandlePositions;
            Vec3f::List m_unselectedFaceHandlePositions;
            Vec3f::List m_selectedHandlePositions;
            
            Renderer::PointHandleRenderer m_handleRenderer;
            Renderer::EdgeRenderer m_edgeRenderer;
            
            bool m_renderStateValid;
        public:
            VertexHandleManager();
            
            const Model::VertexToBrushesMap& unselectedVertexHandles() const;
            const Model::VertexToBrushesMap& selectedVertexHandles() const;
            const Model::VertexToEdgesMap& unselectedEdgeHandles() const;
            const Model::VertexToEdgesMap& selectedEdgeHandles() const;
            const Model::VertexToFacesMap& unselectedFaceHandles() const;
            const Model::VertexToFacesMap& selectedFaceHandles() const;
            
            bool isVertexHandleSelected(const Vec3& position) const;
            bool isEdgeHandleSelected(const Vec3& position) const;
            bool isFaceHandleSelected(const Vec3& position) const;
            
            size_t selectedVertexCount() const;
            size_t totalVertexCount() const;
            size_t selectedEdgeCount() const;
            size_t totalEdgeCount() const;
            size_t selectedFaceCount() const;
            size_t totalSelectedFaceCount() const;
            
            const Model::BrushList& brushes(const Vec3& handlePosition) const;
            const Model::BrushEdgeList& edges(const Vec3& handlePosition) const;
            const Model::BrushFaceList& faces(const Vec3& handlePosition) const;
            
            void addBrush(Model::Brush* brush);
            void addBrushes(const Model::BrushList& brushes);
            void removeBrush(Model::Brush* brush);
            void removeBrushes(const Model::BrushList& brushes);
            void clear();
            
            void selectVertexHandle(const Vec3& position);
            void deselectVertexHandle(const Vec3& position);
            void selectVertexHandles(const Vec3::List& positions);
            void deselectAllVertexHandles();
            
            void selectEdgeHandle(const Vec3& position);
            void deselectEdgeHandle(const Vec3& position);
            void selectEdgeHandles(const Edge3::List& edges);
            void deselectAllEdgeHandles();
            
            void selectFaceHandle(const Vec3& position);
            void deselectFaceHandle(const Vec3& position);
            void selectFaceHandles(const Polygon3::List& faces);
            void deselectAllFaceHandles();
            
            void deselectAllHandles();
            
            void pick(const Ray3& ray, Model::PickResult& pickResult, bool splitMode) const;
            void render(Renderer::RenderContext& renderContext, bool splitMode);
            void renderHighlight(Renderer::RenderContext& renderContext, const Vec3& position);
        private:
            template <typename Element>
            inline bool removeHandle(const Vec3& position, Element* element, std::map<Vec3, std::vector<Element*>, Vec3::LexicographicOrder >& map) {
                typedef std::vector<Element*> List;
                typedef std::map<Vec3, List, Vec3::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = map.find(position);
                if (mapIt == map.end())
                    return false;
                
                List& elements = mapIt->second;
                typename List::iterator listIt = std::find(elements.begin(), elements.end(), element);
                if (listIt == elements.end())
                    return false;
                
                elements.erase(listIt);
                if (elements.empty())
                    map.erase(mapIt);
                return true;
            }
            
            template <typename Element>
            inline size_t moveHandle(const Vec3f& position, std::map<Vec3, std::vector<Element*>, Vec3::LexicographicOrder >& from, std::map<Vec3, std::vector<Element*>, Vec3::LexicographicOrder >& to) {
                typedef std::vector<Element*> List;
                typedef std::map<Vec3, List, Vec3::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = from.find(position);
                if (mapIt == from.end())
                    return 0;
                
                List& fromElements = mapIt->second;
                List& toElements = to[position];
                const size_t elementCount = fromElements.size();
                toElements.insert(toElements.end(), fromElements.begin(), fromElements.end());
                
                from.erase(mapIt);
                return elementCount;
            }

            Model::Hit pickHandle(const Ray3& ray, const Vec3& position, const Model::Hit::HitType type) const;
            void validateRenderState(bool splitMode);
        };
    }
}

#endif /* defined(__TrenchBroom__VertexHandleManager__) */
