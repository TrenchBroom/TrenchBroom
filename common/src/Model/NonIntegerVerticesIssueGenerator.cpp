/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
            IssueQuickFix(NonIntegerVerticesIssue::Type, "Convert vertices to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueArray& issues) const {
                facade->snapVertices(1);
            }
        };

        NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssueGenerator() :
        IssueGenerator(NonIntegerVerticesIssue::Type, "Non-integer vertices") {
            addQuickFix(new NonIntegerVerticesIssueQuickFix());
        }

        void NonIntegerVerticesIssueGenerator::doGenerate(Brush* brush, IssueArray& issues) const {
            for (const BrushVertex* vertex : brush->vertices()) {
                if (!vertex->position().isInteger()) {
                    issues.push_back(new NonIntegerVerticesIssue(brush));
                    return;
                }
            }
        }
    }
}
