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

#include "EmptyAttributeValueIssueGenerator.h"

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
        class EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            std::string m_attributeName;
        public:
            EmptyAttributeValueIssue(AttributableNode* node, const std::string& attributeName) :
            Issue(node),
            m_attributeName(attributeName) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return "Attribute '" + m_attributeName + "' of " + attributableNode->name() + " has an empty value.";
            }

            const std::string& attributeName() const {
                return m_attributeName;
            }
        };

        const IssueType EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssue::Type = Issue::freeType();

        class EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssueQuickFix : public IssueQuickFix {
        public:
            EmptyAttributeValueIssueQuickFix() :
            IssueQuickFix(EmptyAttributeValueIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const EmptyAttributeValueIssue* actualIssue = static_cast<const EmptyAttributeValueIssue*>(issue);
                const std::string& attributeName = actualIssue->attributeName();

                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(attributeName);
            }
        };

        EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssueGenerator() :
        IssueGenerator(EmptyAttributeValueIssue::Type, "Empty property value") {
            addQuickFix(new EmptyAttributeValueIssueQuickFix());
        }

        void EmptyAttributeValueIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->entity().attributes()) {
                if (attribute.value().empty())
                    issues.push_back(new EmptyAttributeValueIssue(node, attribute.name()));
            }
        }
    }
}
