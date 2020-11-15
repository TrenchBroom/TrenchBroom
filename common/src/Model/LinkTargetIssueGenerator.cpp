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

#include "LinkTargetIssueGenerator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class LinkTargetIssueGenerator::LinkTargetIssue : public Issue {
        public:
            friend class LinkTargetIssueQuickFix;
        private:
            const std::string m_name;
        public:
            static const IssueType Type;
        public:
            LinkTargetIssue(AttributableNode* node, const std::string& name) :
            Issue(node),
            m_name(name) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->name() + " has missing target for key '" + m_name + "'";
            }
        };

        const IssueType LinkTargetIssueGenerator::LinkTargetIssue::Type = Issue::freeType();

        class LinkTargetIssueGenerator::LinkTargetIssueQuickFix : public IssueQuickFix {
        private:
            using AttributeNameMap = std::map<std::string, std::vector<Node*>>;
        public:
            LinkTargetIssueQuickFix() :
            IssueQuickFix(LinkTargetIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                const LinkTargetIssue* targetIssue = static_cast<const LinkTargetIssue*>(issue);
                const std::string& attributeName = targetIssue->m_name;

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(attributeName);
            }
        };

        LinkTargetIssueGenerator::LinkTargetIssueGenerator() :
        IssueGenerator(LinkTargetIssue::Type, "Missing entity link source") {
            addQuickFix(new LinkTargetIssueQuickFix());
        }

        void LinkTargetIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            processKeys(node, node->findMissingLinkTargets(), issues);
            processKeys(node, node->findMissingKillTargets(), issues);
        }

        void LinkTargetIssueGenerator::processKeys(AttributableNode* node, const std::vector<std::string>& names, IssueList& issues) const {
            issues.reserve(issues.size() + names.size());
            for (const std::string& name : names) {
                issues.push_back(new LinkTargetIssue(node, name));
            }
        }
    }
}
