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

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "Controller/Tool.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type ClipHandleHit    = 1 << 5;
        }
        
        class ClipHandleHit : public Hit {
        private:
            unsigned int m_index;
        public:
            ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int index);
            bool pickable(Filter& filter) const;
            
            inline unsigned int index() const {
                return m_index;
            }
        };
    }

    namespace Renderer {
        class Vbo;
        class RenderContext;
    }
    
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class ClipFilter : public Model::Filter {
        protected:
            Model::Filter& m_defaultFilter;
        public:
            ClipFilter(Model::Filter& defaultFilter) :
            m_defaultFilter(defaultFilter) {}
            
            virtual inline bool entityVisible(const Model::Entity& entity) const {
                return m_defaultFilter.entityVisible(entity);
            }
            
            virtual inline bool entityPickable(const Model::Entity& entity) const {
                return false;
            }
            
            virtual inline bool brushVisible(const Model::Brush& brush) const {
                return m_defaultFilter.brushVisible(brush);
            }
            
            virtual inline bool brushPickable(const Model::Brush& brush) const {
                return brush.selected() && m_defaultFilter.brushPickable(brush);
            }
            
            virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                return false;
            }
        };

        class ClipTool : public Tool {
        private:
            ClipFilter m_filter;
            Vec3f m_points[3];
            unsigned int m_numPoints;
            int m_hitIndex;
        protected:
            bool handleIsModal(InputState& inputState);
            
//            void handlePick(InputState& inputState);
            bool handleUpdateState(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
        public:
            ClipTool(View::DocumentViewHolder& documentViewHolder);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
