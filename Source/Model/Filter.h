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

#ifndef TrenchBroom_Filter_h
#define TrenchBroom_Filter_h

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"

namespace TrenchBroom {
    namespace Model {
        class Filter {
        public:
            inline bool brushVisible(const Model::Brush& brush) const {
                if (brush.hidden())
                    return false;
                
                return true;
            }
            
            inline bool entityVisible(const Model::Entity& entity) const {
                if (entity.hidden() || entity.worldspawn())
                    return false;
                
                return true;
            }
            
            inline bool brushPickable(const Model::Brush& brush) const {
                if (brush.hidden() || brush.locked())
                    return false;
                
                return true;
            }
            
            inline bool brushVerticesPickable(const Model::Brush& brush) const {
                if (brush.hidden() || brush.locked())
                    return false;
                
                return true;
            }
            
            inline bool entityPickable(const Model::Entity& entity) const {
                if (entity.worldspawn() || entity.hidden() || entity.locked())
                    return false;
                
                EntityDefinition* definition = entity.definition();
                if (definition != NULL && definition->type() == EntityDefinition::BrushEntity && !entity.brushes().empty())
                    return false;
                
                return true;
            }
        };
    }
}

#endif
