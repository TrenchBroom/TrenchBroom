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

#include "FloatVerticesIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class FloatVerticesIssue : public Issue {
        private:
            static const QuickFixType SnapVerticesToIntegerFix = 0;
            static const IssueType Type;
            
            Brush* m_brush;
        public:
            FloatVerticesIssue(Brush* brush) :
            Issue(Type),
            m_brush(brush) {
                addQuickFix(QuickFix(SnapVerticesToIntegerFix, Type, "Snap vertices to integer"));
            }
            
            String description() const {
                return "Brush has non-integer vertices";
            }
            
            void select(View::ControllerSPtr controller) {
                controller->selectObject(*m_brush);
            }
            
            void applyQuickFix(const QuickFixType fixType, View::ControllerSPtr controller) {
            }
        };
        
        const IssueType FloatVerticesIssue::Type = Issue::freeType();
        
        Issue* FloatVerticesIssueGenerator::generate(Brush* brush) const {
            assert(brush != NULL);
            const BrushVertexList& vertices = brush->vertices();
            BrushVertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                const BrushVertex* vertex = *it;
                    if (!vertex->position().isInteger())
                        return new FloatVerticesIssue(brush);
            }
            
            return NULL;
        }
    }
}
