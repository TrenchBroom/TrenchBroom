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

#include "AttributeNameWithDoubleQuotationMarksIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/TransformEntityAttributesQuickFix.h"

#include <kdl/string_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const std::string m_attributeName;
        public:
            AttributeNameWithDoubleQuotationMarksIssue(AttributableNode* node, const std::string& attributeName) :
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
                return "The key of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };

        const IssueType AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue::Type = Issue::freeType();

        AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeNameWithDoubleQuotationMarksIssue::Type, "Invalid entity property keys") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const std::string& name)   { return kdl::str_replace_every(name, "\"", "'"); },
                                                              [] (const std::string& value) { return value; }));
        }

        void AttributeNameWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->entity().attributes()) {
                const std::string& attributeName = attribute.name();
                if (attributeName.find('"') != std::string::npos) {
                    issues.push_back(new AttributeNameWithDoubleQuotationMarksIssue(node, attributeName));
                }
            }
        }
    }
}
