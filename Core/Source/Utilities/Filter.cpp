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

#include "Filter.h"

#include "Controller/Editor.h"
#include "Controller/Options.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Face.h"
#include "Model/Map/Groups.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"

namespace TrenchBroom {
    bool Filter::brushVisible(Model::Brush& brush) {
        const Controller::TransientOptions& options = m_editor.options();
        if (!options.renderBrushes)
            return false;
        
        const Model::Selection& selection = m_editor.map().selection();
        if (options.isolationMode == Controller::IM_DISCARD) {
            if (selection.mode() == Model::TB_SM_FACES) {
                for (unsigned int i = 0; i < brush.faces.size(); i++)
                    if (brush.faces[i]->selected)
                        return true;
                
                return false;
            }
            
            return brush.selected;
        }
        
        Model::GroupManager& groupManager = m_editor.map().groupManager();
        if (groupManager.allGroupsVisible())
            return true;
        
        if (brush.entity->group())
            return groupManager.visible(*brush.entity);
        
        return false;
    }
    
    bool Filter::entityVisible(Model::Entity& entity) {
        const Controller::TransientOptions& options = m_editor.options();
        if (entity.worldspawn() || !options.renderEntities)
            return false;
        
        if (options.isolationMode == Controller::IM_DISCARD)
            return entity.selected();
        
        Model::GroupManager& groupManager = m_editor.map().groupManager();
        if (groupManager.allGroupsVisible())
            return true;
        
        if (entity.group())
            return groupManager.visible(entity);
        
        return true;
    }
    
    bool Filter::brushPickable(Model::Brush& brush) {
        return brushVisible(brush);
    }
    
    bool Filter::brushVerticesPickable(Model::Brush& brush) {
        return true;
    }
    
    bool Filter::entityPickable(Model::Entity& entity) {
        return entityVisible(entity);
    }

}
