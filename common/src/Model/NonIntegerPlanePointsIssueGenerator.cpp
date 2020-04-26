/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "NonIntegerPlanePointsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssue : public Issue {
        public:
            friend class NonIntegerPlanePointsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            explicit NonIntegerPlanePointsIssue(Brush* brush) :
            Issue(brush) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Brush has non-integer plane points";
            }
        };

        const IssueType NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssue::Type = Issue::freeType();

        class NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssueQuickFix : public IssueQuickFix {
        public:
            NonIntegerPlanePointsIssueQuickFix() :
            IssueQuickFix(NonIntegerPlanePointsIssue::Type, "Convert plane points to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->findPlanePoints();
            }
        };

        NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssueGenerator() :
        IssueGenerator(NonIntegerPlanePointsIssue::Type, "Non-integer plane points") {
            // Disabled until findPlanePoints() is fixed, see: https://github.com/kduske/TrenchBroom/issues/2780
            // addQuickFix(new NonIntegerPlanePointsIssueQuickFix());
        }

        void NonIntegerPlanePointsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            for (const BrushFace* face : brush->faces()) {
                const BrushFace::Points& points = face->points();
                for (size_t i = 0; i < 3; ++i) {
                    const vm::vec3& point = points[i];
                    if (!vm::is_integral(point)) {
                        issues.push_back(new NonIntegerPlanePointsIssue(brush));
                        return;
                    }
                }
            }
        }
    }
}
