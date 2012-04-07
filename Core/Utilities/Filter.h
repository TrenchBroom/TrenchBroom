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

#include "Entity.h"
#include "Brush.h"
#include "Groups.h"
#include "Selection.h"

namespace TrenchBroom {
    
    class Filter {
    private:
    public:
        bool brushVisible(Brush& brush) { return true; }
        bool entityVisible(Entity& entity) { return true; }
        bool brushPickable(Brush& brush) { return true; }
        bool brushVerticesPickable(Brush& brush) { return true; }
        bool entityPickable(Entity& entity) { return true; }
    };
    
}

#endif
