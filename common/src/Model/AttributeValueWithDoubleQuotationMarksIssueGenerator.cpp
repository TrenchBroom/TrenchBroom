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

#include "AttributeValueWithDoubleQuotationMarksIssueGenerator.h"

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
        class AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const std::string m_attributeName;
        public:
            AttributeValueWithDoubleQuotationMarksIssue(AttributableNode* node, const std::string& attributeName) :
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
                return "The value of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };

        const IssueType AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue::Type = Issue::freeType();

        AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeValueWithDoubleQuotationMarksIssue::Type, "Invalid entity property values") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const std::string& name)   { return name; },
                                                              [] (const std::string& value) { return kdl::str_replace_every(value, "\"", "'"); }));
        }

        void AttributeValueWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->entity().attributes()) {
                const std::string& attributeName = attribute.name();
                const std::string& attributeValue = attribute.value();
                if (attributeValue.find('"') != std::string::npos) {
                    issues.push_back(new AttributeValueWithDoubleQuotationMarksIssue(node, attributeName));
                }
            }
        }
    }
}
