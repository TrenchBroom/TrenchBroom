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

#include "OutlineTracer.h"

#include "CollectionUtils.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace Renderer {
        OutlineTracer::Position::Position() :
        x(0.0),
        count(0) {}
        
        OutlineTracer::Position::Position(FloatType i_x, size_t i_count) :
        x(i_x),
        count(i_count) {}

        bool OutlineTracer::Position::operator== (const Position& other) const {
            return Math::eq(x, other.x);
        }

        bool OutlineTracer::Position::operator< (const Position& other) const {
            return Math::lt(x, other.x);
        }

        Edge3::List OutlineTracer::edges() const {
            Edge3::List result;
            
            EdgeMap::const_iterator mapIt, mapEnd;
            Position::List::const_iterator posIt, posEnd;
            
            for (mapIt = m_edges.begin(), mapEnd = m_edges.end(); mapIt != mapEnd; ++mapIt) {
                const Line3& line = mapIt->first;
                const Vec3& offset = line.point;
                const Vec3& dir = line.direction;
                
                const Position::List& positions = mapIt->second;
                for (posIt = positions.begin(), posEnd = positions.end(); posIt != posEnd; ++posIt) {
                    const Position& start = *posIt;
                    assert(start.count == 1);

                    ++posIt;
                    assert(posIt != posEnd);
                    const Position& end = *posIt;
                    assert(end.count != 1);
                    
                    result.push_back(Edge3(offset + start.x * dir, offset + end.x * dir));
                }
            }
            
            return result;
        }

        void OutlineTracer::addEdge(const Edge3& edge) {
            const Vec3 anchor = edge.start;
            const Vec3 direction = (edge.end - edge.start).normalized();
            const Line3 line = Line3(anchor, direction).makeCanonical();

            EdgeMap::iterator it = MapUtils::findOrInsert(m_edges, line);
            assert(m_edges.count(line) > 0);
            assert(it->second.empty() || m_edges.count(line) != 0);
            addEdge(edge, line, it->second);
            assert(m_edges.count(line) > 0);
        }

        void OutlineTracer::clear() {
            m_edges.clear();
        }

        void OutlineTracer::addEdge(const Edge3& edge, const Line3& line, Position::List& positions) {
            FloatType left = edge.start.dot(line.direction);
            FloatType right = edge.end.dot(line.direction);
            if (left > right)
                std::swap(left, right);
            
            Position leftPos(left, 0);
            Position rightPos(right, 0);
            
            typedef Position::List::iterator Iter;
            Iter leftIP  = findInsertPos(leftPos, positions);
            Iter rightIP = findInsertPos(rightPos, positions);
            
            setCount(leftIP, leftPos, positions);
            setCount(rightIP, rightPos, positions);
            
            Iter start;
            if (replace(leftIP, leftPos, positions))
                start = leftIP;
            else
                start = positions.insert(leftIP, leftPos);
            
            Iter end;
            if (replace(rightIP, rightPos, positions))
                end = rightIP;
            else
                end = positions.insert(rightIP, rightPos);
            
            fixCounts(start, end);
//            print(positions);
            mergeEdges(start, end, positions);
//            print(positions);
        }

        OutlineTracer::Position::List::iterator OutlineTracer::findInsertPos(const Position& position, Position::List& positions) const {
            return std::lower_bound(positions.begin(), positions.end(), position);
        }

        void OutlineTracer::setCount(Position::List::iterator insertPos, Position& position, Position::List& positions) const {
            typedef Position::List::iterator Iter;

            if (insertPos != positions.begin() && insertPos != positions.end()) {
                Iter prev = insertPos;
                --prev;
                position.count = prev->count;
            }
        }

        bool OutlineTracer::replace(Position::List::iterator it, const Position& position, Position::List& positions) const {
            return it != positions.end() && Math::eq(it->x, position.x);
        }

        void OutlineTracer::fixCounts(Position::List::iterator it, Position::List::iterator end) {
            while (it != end) {
                it->count++;
                ++it;
            }
        }
        
        void OutlineTracer::mergeEdges(Position::List::iterator it, Position::List::iterator end, Position::List& positions) {
            size_t last;
            if (it != positions.begin()) {
                --it;
                last = it->count;
                ++it;
            } else {
                last = 0;
            }
            
            if (end != positions.end()) {
                ++end;
                if (end != positions.end())
                    ++end;
            }
            
            while (it != end) {
                const size_t current = it->count;
                if ((last == current) ||
                    (last != 1 && current != 1)) {
                    it = positions.erase(it);
                } else {
                    last = current;
                    ++it;
                }
            }
        }
        
        void OutlineTracer::print(const Position::List& positions) const {
            Position::List::const_iterator it, end;
            for (it = positions.begin(), end = positions.end(); it != end; ++it)
                std::cout << it->x << ":" << it->count << "; ";
            std::cout << std::endl;
        }
    }
}
