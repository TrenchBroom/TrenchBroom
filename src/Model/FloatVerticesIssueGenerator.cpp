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
#include "Model/QuickFix.h"
#include "Model/SharedQuickFixes.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class FloatVerticesIssue : public BrushIssue {
        public:
            static const IssueType Type;
        public:
            FloatVerticesIssue(Brush* brush) :
            BrushIssue(Type, brush) {
                addSharedQuickFix(SnapVerticesToIntegerQuickFix::instance());
            }
            
            String description() const {
                return "Brush has non-integer vertices";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == FindIntegerPlanePointsQuickFix::Type)
                    static_cast<const SnapVerticesToIntegerQuickFix*>(quickFix)->apply(brush(), controller);
            }
        };
        
        const IssueType FloatVerticesIssue::Type = Issue::freeType();
        
        IssueType FloatVerticesIssueGenerator::type() const {
            return FloatVerticesIssue::Type;
        }
        
        const String& FloatVerticesIssueGenerator::description() const {
            static const String description("Non-integer vertices");
            return description;
        }

        Issue* FloatVerticesIssueGenerator::generate(Brush* brush) const {
            assert(brush != NULL);
            const BrushVertexList& vertices = brush->vertices();
            BrushVertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                const BrushVertex* vertex = *it;
                    if (!vertex->position.isInteger())
                        return new FloatVerticesIssue(brush);
            }
            
            return NULL;
        }
    }
}
