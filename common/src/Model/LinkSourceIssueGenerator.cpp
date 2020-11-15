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

#include "LinkSourceIssueGenerator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityAttributes.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        class LinkSourceIssueGenerator::LinkSourceIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            explicit LinkSourceIssue(AttributableNode* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->name() + " has unused targetname key";
            }
        };

        const IssueType LinkSourceIssueGenerator::LinkSourceIssue::Type = Issue::freeType();

        class LinkSourceIssueGenerator::LinkSourceIssueQuickFix : public IssueQuickFix {
        public:
            LinkSourceIssueQuickFix() :
            IssueQuickFix(LinkSourceIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(AttributeNames::Targetname);
            }
        };

        LinkSourceIssueGenerator::LinkSourceIssueGenerator() :
        IssueGenerator(LinkSourceIssue::Type, "Missing entity link source") {
            addQuickFix(new LinkSourceIssueQuickFix());
        }

        void LinkSourceIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->hasMissingSources())
                issues.push_back(new LinkSourceIssue(node));
        }
    }
}
