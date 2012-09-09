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
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

class wxEvtHandler;

namespace TrenchBroom {
    namespace Controller {
        class CameraFilter : public Model::Filter {
        public:
            virtual inline bool brushPickable(const Model::Brush& brush) const {
                if (brush.hidden())
                    return false;
                
                return true;
            }
            
            virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                return false;
            }
            
            virtual inline bool entityPickable(const Model::Entity& entity) const {
                if (entity.worldspawn() || entity.hidden())
                    return false;
                
                Model::EntityDefinition* definition = entity.definition();
                if (definition != NULL && definition->type() == Model::EntityDefinition::BrushEntity && !entity.brushes().empty())
                    return false;
                
                return true;
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
        public:
            CameraTool(Model::MapDocument& document, View::EditorView& view) : Tool(document, view), m_orbit(false) {}
            
            bool handleScrolled(InputEvent& event);
            bool handleBeginDrag(InputEvent& event);
            bool handleDrag(InputEvent& event);
            void handleEndDrag(InputEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__CameraTool__) */
