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

#ifndef __TrenchBroom__BrushEdge__
#define __TrenchBroom__BrushEdge__

namespace TrenchBroom {
    namespace Model {
        class BrushVertex;
        
        class BrushEdge {
        private:
            BrushVertex* m_start;
            BrushVertex* m_end;
        public:
            BrushEdge(BrushVertex* start, BrushVertex* end);
            ~BrushEdge();
            
            const BrushVertex* start() const;
            const BrushVertex* end() const;
        };
    }
}

#endif /* defined(__TrenchBroom__BrushEdge__) */
