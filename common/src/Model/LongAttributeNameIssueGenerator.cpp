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

#include "LongAttributeNameIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/MapFacade.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class LongAttributeNameIssueGenerator::LongAttributeNameIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const std::string m_attributeName;
        public:
            LongAttributeNameIssue(AttributableNode* node, const std::string& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const std::string& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Entity property key '" + m_attributeName.substr(0, 8) + "...' is too long.";
            }
        };

        const IssueType LongAttributeNameIssueGenerator::LongAttributeNameIssue::Type = Issue::freeType();

        LongAttributeNameIssueGenerator::LongAttributeNameIssueGenerator(const size_t maxLength) :
        IssueGenerator(LongAttributeNameIssue::Type, "Long entity property keys"),
        m_maxLength(maxLength) {
            addQuickFix(new RemoveEntityAttributesQuickFix(LongAttributeNameIssue::Type));
        }

        void LongAttributeNameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->entity().attributes()) {
                const std::string& attributeName = attribute.name();
                if (attributeName.size() >= m_maxLength) {
                    issues.push_back(new LongAttributeNameIssue(node, attributeName));
                }
            }
        }
    }
}
