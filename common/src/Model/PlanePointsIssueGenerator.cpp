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

#include "PlanePointsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>
#include <map>

namespace TrenchBroom {
    namespace Model {
        class PlanePointsIssueGenerator::PlanePointsIssue : public Issue {
        public:
            friend class PlanePointsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            PlanePointsIssue(Brush* brush) :
            Issue(brush) {}
            
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return "Brush has non-integer plane points";
            }
        };
        
        const IssueType PlanePointsIssueGenerator::PlanePointsIssue::Type = Issue::freeType();

        class PlanePointsIssueGenerator::PlanePointsIssueQuickFix : public IssueQuickFix {
        public:
            PlanePointsIssueQuickFix() :
            IssueQuickFix("Convert plane points to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->findPlanePoints();
            }
        };
        
        PlanePointsIssueGenerator::PlanePointsIssueGenerator() :
        IssueGenerator(PlanePointsIssue::Type, "Non-integer plane points") {
            addQuickFix(new PlanePointsIssueQuickFix());
        }

        void PlanePointsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                const BrushFace::Points& points = face->points();
                for (size_t i = 0; i < 3; ++i) {
                    const Vec3& point = points[i];
                    if (!point.isInteger()) {
                        issues.push_back(new PlanePointsIssue(brush));
                        return;
                    }
                }
            }
        }
    }
}
