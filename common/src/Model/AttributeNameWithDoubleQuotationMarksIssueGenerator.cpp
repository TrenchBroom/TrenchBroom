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

#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/TransformEntityAttributesQuickFix.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            AttributeNameWithDoubleQuotationMarksIssue(AttributableNode* node, const AttributeName& attributeName) :
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
                return "The key of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };
        
        const IssueType AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue::Type = Issue::freeType();
        
        AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeNameWithDoubleQuotationMarksIssue::Type, "Invalid entity property keys") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const AttributeName& name)   { return StringUtils::replaceAll(name, "\"", "'"); },
                                                              [] (const AttributeValue& value) { return value; }));
        }
        
        void AttributeNameWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                if (attributeName.find('"') != String::npos)
                    issues.push_back(new AttributeNameWithDoubleQuotationMarksIssue(node, attributeName));
            }
        }
    }
}
