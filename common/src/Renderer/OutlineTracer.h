/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__OutlineTracer__
#define __TrenchBroom__OutlineTracer__

#include "TrenchBroom.h"
#include "VecMath.h"

#include <list>
#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class OutlineTracer {
        private:
            struct Position {
                typedef std::list<Position> List;

                FloatType x;
                size_t count;
                
                Position();
                Position(FloatType i_x, size_t i_count);

                bool operator== (const Position& other) const;
                bool operator< (const Position& other) const;
            };

            typedef std::map<Line3, Position::List> EdgeMap;
            EdgeMap m_edges;
        public:
            Edge3::List edges() const;
            
            void addEdge(const Edge3& edge);
            void clear();
        private:
            void addEdge(const Edge3& edge, const Line3& line, Position::List& positions);
            Position::List::iterator findInsertPos(const Position& position, Position::List& positions) const;
            void setCount(Position::List::iterator insertPos, Position& position, Position::List& positions) const;
            bool replace(Position::List::iterator it, const Position& position, Position::List& positions) const;
            void fixCounts(Position::List::iterator it, Position::List::iterator end);
            void mergeEdges(Position::List::iterator it, Position::List::iterator end, Position::List& positions);
            void print(const Position::List& positions) const;
        };
    }
}

#endif /* defined(__TrenchBroom__OutlineTracer__) */
