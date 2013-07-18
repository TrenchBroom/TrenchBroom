/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Filter__
#define __TrenchBroom__Filter__

#include "Model/Brush.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        class Filter {
        public:
            bool passes(Entity::Ptr entity) const;
            bool passes(Brush::Ptr brush) const;
        };
        
        class SelectedFilter {
        public:
            bool passes(Entity::Ptr entity) const;
            bool passes(Brush::Ptr brush) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Filter__) */
