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

#ifndef __TrenchBroom__CameraTool__
#define __TrenchBroom__CameraTool__

#include "Controller/Tool.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class CameraFilter : public Model::Filter {
        protected:
            Model::Filter& m_defaultFilter;
        public:
            CameraFilter(Model::Filter& defaultFilter) :
            m_defaultFilter(defaultFilter) {}
            
            virtual inline bool entityVisible(const Model::Entity& entity) const {
                return m_defaultFilter.entityVisible(entity);
            }
            
            virtual bool entityPickable(const Model::Entity& entity) const;
            
            virtual inline bool brushVisible(const Model::Brush& brush) const {
                return m_defaultFilter.brushVisible(brush);
            }
            
            virtual inline bool brushPickable(const Model::Brush& brush) const {
                return brushVisible(brush);
            }
            
            virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                return false;
            }
        };
        

        class CameraTool : public Tool {
        private:
            Vec3f m_orbitCenter;
            bool m_orbit;
            CameraFilter m_filter;

            float lookSpeed(bool vertical);
            float panSpeed(bool vertical);
            float moveSpeed();
        protected:
            void handleScroll(InputState& inputState);

            bool handleStartDrag(InputState& inputState);
            bool handleDrag(InputState& inputState);
            void handleEndDrag(InputState& inputState);
        public:
            CameraTool(View::DocumentViewHolder& documentViewHolder) :
            Tool(documentViewHolder, false),
            m_orbit(false),
            m_filter(CameraFilter(documentViewHolder.view().filter())) {}
        };
    }
}

#endif /* defined(__TrenchBroom__CameraTool__) */
