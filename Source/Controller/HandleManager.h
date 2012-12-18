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

#ifndef __TrenchBroom__HandleManager__
#define __TrenchBroom__HandleManager__

#include "Model/Brush.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/Picker.h"
#include "Renderer/RenderTypes.h"
#include "Utility/VecMath.h"

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
            
            Renderer::PointHandleRendererPtr m_selectedHandleRenderer;
            Renderer::PointHandleRendererPtr m_unselectedHandleRenderer;
            bool m_renderStateValid;
            
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
            HandleManager();
            
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
            
            inline bool vertexHandleSelected(const Vec3f& position) {
                return m_selectedVertexHandles.find(position) != m_selectedVertexHandles.end();
            }
            
            inline bool edgeHandleSelected(const Vec3f& position) {
                return m_selectedEdgeHandles.find(position) != m_selectedEdgeHandles.end();
            }
            
            
            const Model::EdgeList& edges(const Vec3f& handlePosition) const;

            void add(Model::Brush& brush);
            void add(const Model::BrushList& brushes);
            void remove(Model::Brush& brush);
            void remove(const Model::BrushList& brushes);
            void clear();

            bool selectVertexHandle(const Vec3f& position);
            bool deselectVertexHandle(const Vec3f& position);
            bool selectVertexHandles(const Vec3f::Set& positions);
            void deselectVertexHandles();

            bool selectEdgeHandle(const Vec3f& position);
            bool deselectEdgeHandle(const Vec3f& position);
            void deselectEdgeHandles();
            
            void deselectAll();
            
            void saveSelection();
            void clearSavedSelection();
            void restoreSavedSelection();
            
            void pick(const Ray& ray, Model::PickResult& pickResult) const;
            void render(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            
            inline void invalidateRenderState() {
                m_renderStateValid = false;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__HandleManager__) */
