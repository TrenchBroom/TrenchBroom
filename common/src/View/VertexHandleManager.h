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

#ifndef TrenchBroom_VertexHandleManager
#define TrenchBroom_VertexHandleManager

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometry.h"
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/PointGuideRenderer.h"
#include "View/ViewTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class VertexHandleManager {
        public:
            static const Model::Hit::HitType VertexHandleHit;
            static const Model::Hit::HitType EdgeHandleHit;
            static const Model::Hit::HitType FaceHandleHit;
        private:
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
            
            Vec3::List m_unselectedVertexHandlePositions;
            Vec3::List m_unselectedEdgeHandlePositions;
            Vec3::List m_unselectedFaceHandlePositions;
            Vec3::List m_selectedHandlePositions;
            
            Vec3f::List m_edgeVertices;
            Renderer::PointGuideRenderer m_guideRenderer;
            
            bool m_renderStateValid;
        public:
            VertexHandleManager(View::MapDocumentWPtr document);
            
            const Model::VertexToBrushesMap& unselectedVertexHandles() const;
            const Model::VertexToBrushesMap& selectedVertexHandles() const;
            const Model::VertexToEdgesMap& unselectedEdgeHandles() const;
            const Model::VertexToEdgesMap& selectedEdgeHandles() const;
            const Model::VertexToFacesMap& unselectedFaceHandles() const;
            const Model::VertexToFacesMap& selectedFaceHandles() const;
            
            Vec3::List vertexHandlePositions() const;
            Vec3::List edgeHandlePositions() const;
            Vec3::List faceHandlePositions() const;
            
            Vec3::List unselectedVertexHandlePositions() const;
            Vec3::List unselectedEdgeHandlePositions() const;
            Vec3::List unselectedFaceHandlePositions() const;
            
            Vec3::List selectedVertexHandlePositions() const;
            Vec3::List selectedEdgeHandlePositions() const;
            Vec3::List selectedFaceHandlePositions() const;
            
            bool isHandleSelected(const Vec3& position) const;
            bool isVertexHandleSelected(const Vec3& position) const;
            bool isEdgeHandleSelected(const Vec3& position) const;
            bool isFaceHandleSelected(const Vec3& position) const;
            
            size_t selectedVertexCount() const;
            size_t totalVertexCount() const;
            size_t selectedEdgeCount() const;
            size_t totalEdgeCount() const;
            size_t selectedFaceCount() const;
            size_t totalSelectedFaceCount() const;
            
            Model::BrushSet selectedBrushes() const;
            const Model::BrushSet& brushes(const Vec3& handlePosition) const;
            const Model::BrushEdgeSet& edges(const Vec3& handlePosition) const;
            const Model::BrushFaceSet& faces(const Vec3& handlePosition) const;
            
            void addBrush(Model::Brush* brush);
            void removeBrush(Model::Brush* brush);
            
            template <typename I>
            void addBrushes(I cur, I end) {
                while (cur != end)
                    addBrush(*cur++);
            }
            
            template <typename I>
            void removeBrushes(I cur, I end) {
                while (cur != end)
                    removeBrush(*cur++);
            }
            
            void clear();
            
            void selectVertexHandle(const Vec3& position);
            void deselectVertexHandle(const Vec3& position);
            void toggleVertexHandle(const Vec3& position);
            void selectVertexHandles(const Vec3::List& positions);
            void deselectAllVertexHandles();
            void toggleVertexHandles(const Vec3::List& positions);
            
            void selectEdgeHandle(const Vec3& position);
            void deselectEdgeHandle(const Vec3& position);
            void toggleEdgeHandle(const Vec3& position);
            void selectEdgeHandles(const Edge3::List& edges);
            void deselectAllEdgeHandles();
            void toggleEdgeHandles(const Vec3::List& positions);
            
            void selectFaceHandle(const Vec3& position);
            void deselectFaceHandle(const Vec3& position);
            void toggleFaceHandle(const Vec3& position);
            void selectFaceHandles(const Polygon3::List& faces);
            void deselectAllFaceHandles();
            void toggleFaceHandles(const Vec3::List& positions);
            
            bool hasSelectedHandles() const;
            void deselectAllHandles();
            
            void reselectVertexHandles(const Model::BrushSet& brushes, const Vec3::List& positions, FloatType maxDistance);
            void reselectEdgeHandles(const Model::BrushSet& brushes, const Vec3::List& positions, FloatType maxDistance);
            void reselectFaceHandles(const Model::BrushSet& brushes, const Vec3::List& positions, FloatType maxDistance);
            
            void pick(const Ray3& ray, const Renderer::Camera& camera, Model::PickResult& pickResult, bool splitMode) const;
            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, bool splitMode);
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position);
            void renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition);
            void renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition);
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position);
        private:
            template <typename Element>
            inline bool removeHandle(const Vec3& position, Element* element, std::map<Vec3, std::set<Element*>, Vec3::LexicographicOrder >& map) {
                typedef std::set<Element*> Set;
                typedef std::map<Vec3, Set, Vec3::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = map.find(position);
                if (mapIt == map.end())
                    return false;
                
                Set& elements = mapIt->second;
                typename Set::iterator setIt = elements.find(element);
                if (setIt == elements.end())
                    return false;
                
                elements.erase(setIt);
                if (elements.empty())
                    map.erase(mapIt);
                return true;
            }
            
            template <typename Element>
            inline size_t moveHandle(const Vec3& position, std::map<Vec3, std::set<Element*>, Vec3::LexicographicOrder >& from, std::map<Vec3, std::set<Element*>, Vec3::LexicographicOrder >& to) {
                typedef std::set<Element*> Set;
                typedef std::map<Vec3, Set, Vec3::LexicographicOrder> Map;
                
                typename Map::iterator mapIt = from.find(position);
                if (mapIt == from.end())
                    return 0;
                
                Set& fromElements = mapIt->second;
                Set& toElements = to[position];
                const size_t elementCount = fromElements.size();
                toElements.insert(fromElements.begin(), fromElements.end());
                
                from.erase(mapIt);
                return elementCount;
            }

            template <typename T, typename O>
            void handlePositions(const std::map<Vec3, T, O>& handles, Vec3::List& result) const {
                result.reserve(result.size() + handles.size());
                
                typename std::map<Vec3, T, O>::const_iterator it, end;
                for (it = handles.begin(), end = handles.end(); it != end; ++it) {
                    const Vec3& position = it->first;
                    result.push_back(position);
                }
            }

            Vec3::List findVertexHandlePositions(const Model::BrushSet& brushes, const Vec3& query, FloatType maxDistance);
            Vec3::List findEdgeHandlePositions(const Model::BrushSet& brushes, const Vec3& query, FloatType maxDistance);
            Vec3::List findFaceHandlePositions(const Model::BrushSet& brushes, const Vec3& query, FloatType maxDistance);
            
            Model::Hit pickHandle(const Ray3& ray, const Renderer::Camera& camera, const Vec3& position, Model::Hit::HitType type) const;
            void validateRenderState(bool splitMode);
        };
    }
}

#endif /* defined(TrenchBroom_VertexHandleManager) */
