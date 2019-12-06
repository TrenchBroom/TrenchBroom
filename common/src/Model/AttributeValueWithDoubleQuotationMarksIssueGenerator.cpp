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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/TransformEntityAttributesQuickFix.h"

#include <kdl/string_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            AttributeValueWithDoubleQuotationMarksIssue(AttributableNode* node, const AttributeName& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const AttributeName& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "The value of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };

        const IssueType AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue::Type = Issue::freeType();

        AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeValueWithDoubleQuotationMarksIssue::Type, "Invalid entity property values") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const AttributeName& name)   { return name; },
                                                              [] (const AttributeValue& value) { return kdl::str_replace_every(value, "\"", "'"); }));
        }

        void AttributeValueWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                const AttributeValue& attributeValue = attribute.value();
                if (attributeValue.find('"') != String::npos)
                    issues.push_back(new AttributeValueWithDoubleQuotationMarksIssue(node, attributeName));
            }
        }
    }
}
