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
        class EntityFigure;
        class BoundsGuideFigure;
    }
    
    namespace Controller {
        class Editor;
        
        class DragEntityTargetTool : public DragTargetTool {
        protected:
            Renderer::EntityFigure* m_entityFigure;
            Renderer::BoundsGuideFigure* m_guideFigure;
            BBox m_bounds;
            
            void deleteFigures();
            void updateFigures(const DragInfo& info, const Model::EntityDefinition& definition);
        public:
            DragEntityTargetTool(Editor& editor) : DragTargetTool(editor), m_entityFigure(NULL), m_guideFigure(NULL) {}
            virtual ~DragEntityTargetTool() {}
            
            virtual bool accepts(const DragInfo& info);
            virtual bool activate(const DragInfo& info);
            virtual void deactivate(const DragInfo& info);
            virtual bool move(const DragInfo& info);
            virtual bool drop(const DragInfo& info);
        };
    }
}

#endif
