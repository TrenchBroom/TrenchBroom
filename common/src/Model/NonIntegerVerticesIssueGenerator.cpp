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

#include "NonIntegerVerticesIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssue : public Issue {
        public:
            friend class NonIntegerVerticesIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            NonIntegerVerticesIssue(Brush* brush) :
            Issue(brush) {}
            
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return "Brush has non-integer vertices";
            }
        };

        const IssueType NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssue::Type = Issue::freeType();
        
        class NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssueQuickFix : public IssueQuickFix {
        public:
            NonIntegerVerticesIssueQuickFix() :
            IssueQuickFix("Convert vertices to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->snapVertices();
            }
        };

        NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssueGenerator() :
        IssueGenerator(NonIntegerVerticesIssue::Type, "Non-integer vertices") {
            addQuickFix(new NonIntegerVerticesIssueQuickFix());
        }

        void NonIntegerVerticesIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            const Brush::VertexList vertices = brush->vertices();
            Brush::VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                const BrushVertex* vertex = *it;
                if (!vertex->position().isInteger()) {
                    issues.push_back(new NonIntegerVerticesIssue(brush));
                    return;
                }
            }
        }
    }
}
