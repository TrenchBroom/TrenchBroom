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

#ifndef __TrenchBroom__EntityDragTargetTool__
#define __TrenchBroom__EntityDragTargetTool__

#include "Controller/DragTargetTool.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class EntityFigure;
    }
    
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class EntityDragTargetTool : public DragTargetTool {
        protected:
            Model::Entity* m_entity;
            Renderer::EntityFigure* m_entityFigure;
            
            void updateFigure(InputEvent& event);
            
            bool handleDragEnter(InputEvent& event, const String& payload);
            void handleDragMove(InputEvent& event);
            void handleDragLeave();
            bool handleDrop(InputEvent& event);
        public:
            EntityDragTargetTool(View::DocumentViewHolder& documentViewHolder);
            ~EntityDragTargetTool();
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDragTargetTool__) */
