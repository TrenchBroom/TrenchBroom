/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class LongAttributeNameIssueGenerator::LongAttributeNameIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            LongAttributeNameIssue(AttributableNode* node, const AttributeName& attributeName) :
            Issue(node),
            m_attributeName(attributeName) {}
            
            const AttributeName& attributeName() const {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return "Entity property key '" + m_attributeName.substr(0, 8) + "...' is too long.";
            }
        };
        
        const IssueType LongAttributeNameIssueGenerator::LongAttributeNameIssue::Type = Issue::freeType();
        
        class LongAttributeNameIssueGenerator::LongAttributeNameIssueQuickFix : public IssueQuickFix {
        private:
        public:
            LongAttributeNameIssueQuickFix() :
            IssueQuickFix(LongAttributeNameIssue::Type, "Delete properties") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const {
                const PushSelection push(facade);
                
                const LongAttributeNameIssue* attrIssue = static_cast<const LongAttributeNameIssue*>(issue);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.
                
                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(attrIssue->attributeName());
            }
        };
        
        LongAttributeNameIssueGenerator::LongAttributeNameIssueGenerator(const size_t maxLength) :
        IssueGenerator(LongAttributeNameIssue::Type, "Missing entity classname"),
        m_maxLength(maxLength) {
            addQuickFix(new LongAttributeNameIssueQuickFix());
        }
        
        void LongAttributeNameIssueGenerator::doGenerate(AttributableNode* node, IssueArray& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                if (attributeName.size() >= m_maxLength)
                    issues.push_back(new LongAttributeNameIssue(node, attributeName));
            }
        }
    }
}
