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

#include "MissingClassnameIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class MissingClassnameIssueGenerator::MissingClassnameIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            explicit MissingClassnameIssue(AttributableNode* node) :
            Issue(node) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Entity has no classname property";
            }
        };

        const IssueType MissingClassnameIssueGenerator::MissingClassnameIssue::Type = Issue::freeType();

        class MissingClassnameIssueGenerator::MissingClassnameIssueQuickFix : public IssueQuickFix {
        public:
            MissingClassnameIssueQuickFix() :
            IssueQuickFix(MissingClassnameIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        MissingClassnameIssueGenerator::MissingClassnameIssueGenerator() :
        IssueGenerator(MissingClassnameIssue::Type, "Missing entity classname") {
            addQuickFix(new MissingClassnameIssueQuickFix());
        }

        void MissingClassnameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (!node->entity().hasAttribute(AttributeNames::Classname))
                issues.push_back(new MissingClassnameIssue(node));
        }
    }
}
