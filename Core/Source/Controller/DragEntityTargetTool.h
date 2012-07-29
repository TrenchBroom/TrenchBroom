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

#ifndef TrenchBroom_DragEntityTargetTool_h
#define TrenchBroom_DragEntityTargetTool_h

#include "Controller/DragTargetTool.h"

namespace TrenchBroom {
    namespace Model {
        class EntityDefinition;
    }
    
    namespace Renderer {
        class DragEntityTargetToolFigure;
    }
    
    namespace Controller {
        class Editor;
        
        class DragEntityTargetTool : public DragTargetTool {
        protected:
            bool m_figureCreated;
            Model::EntityDefinition* m_entityDefinition;
            Vec3f m_position;
            BBox m_bounds;
            
            void update(const DragInfo& info);
        public:
            DragEntityTargetTool(Editor& editor) : DragTargetTool(editor), m_figureCreated(false), m_entityDefinition(NULL) {}
            virtual ~DragEntityTargetTool() {}
            
            virtual bool accepts(const DragInfo& info);
            virtual bool handleActivate(const DragInfo& info);
            virtual void handleDeactivate(const DragInfo& info);
            virtual bool handleMove(const DragInfo& info);
            virtual bool handleDrop(const DragInfo& info);
            
            Model::EntityDefinition* entityDefinition() {
                return m_entityDefinition;
            }
            
            const BBox& bounds() {
                return m_bounds;
            }
            
            const Vec3f& position() {
                return m_position;
            }
        };
    }
}

#endif
