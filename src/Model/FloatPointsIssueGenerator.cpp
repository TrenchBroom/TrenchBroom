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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FloatPointsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/Object.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class FloatPointsIssue : public Issue {
        private:
            Brush* m_brush;
        public:
            FloatPointsIssue(Brush* brush) :
            m_brush(brush) {}
            
            String asString() const {
                return "Brush has floating point plane points";
            }
        };
        
        Issue* FloatPointsIssueGenerator::generate(Brush* brush) const {
            assert(brush != NULL);
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                const BrushFace::Points& points = face->points();
                for (size_t i = 0; i < 3; ++i) {
                    const Vec3& point = points[i];
                    if (!point.isInteger())
                        return new FloatPointsIssue(brush);
                }
            }
            
            return NULL;
        }
    }
}
