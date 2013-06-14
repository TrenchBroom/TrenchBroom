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

#ifndef __TrenchBroom__BrushVertex__
#define __TrenchBroom__BrushVertex__

#include "VecMath.h"
#include "TrenchBroom.h"
#include "Model/BrushVertex.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushVertex {
        public:
            typedef std::vector<BrushVertex*> List;
            static const List EmptyList;
            
            typedef enum {
                Drop,
                Keep,
                Undecided,
                New
            } Mark;
        private:
            Vec3 m_position;
            Mark m_mark;
        public:
            BrushVertex(const Vec3& position);
            
            inline const Vec3& position() const {
                return m_position;
            }
            
            inline Mark mark() const {
                return m_mark;
            }
            
            void updateMark(const Plane3& plane);
        };
        
        inline BrushVertex::List::iterator findBrushVertex(BrushVertex::List& vertices, const Vec3& position) {
            BrushVertex::List::iterator it = vertices.begin();
            const BrushVertex::List::iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }

        inline BrushVertex::List::const_iterator findBrushVertex(const BrushVertex::List& vertices, const Vec3& position) {
            BrushVertex::List::const_iterator it = vertices.begin();
            const BrushVertex::List::const_iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }
    }
}

#endif /* defined(__TrenchBroom__BrushVertex__) */
