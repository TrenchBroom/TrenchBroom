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

#include "TransformEntityAttributesQuickFix.h"

namespace TrenchBroom {
    namespace Model {
        TransformEntityAttributesQuickFix::TransformEntityAttributesQuickFix(const IssueType issueType, const String& description, const NameTransform& nameTransform, const ValueTransform& valueTransform) :
        IssueQuickFix(issueType, description),
        m_nameTransform(nameTransform),
        m_valueTransform(valueTransform) {}

        void TransformEntityAttributesQuickFix::doApply(MapFacade* facade, const Issue* issue) const {
            const PushSelection push(facade);
            
            const AttributeIssue* attrIssue = static_cast<const AttributeIssue*>(issue);
            const AttributeName& oldName = attrIssue->attributeName();
            const AttributeValue& oldValue = attrIssue->attributeValue();
            const AttributeName newName = m_nameTransform(oldName);
            const AttributeValue newValue = m_valueTransform(oldValue);
            
            // If world node is affected, the selection will fail, but if nothing is selected,
            // the removeAttribute call will correctly affect worldspawn either way.
            
            facade->deselectAll();
            facade->select(issue->node());
            
            if (newName.empty()) {
                facade->removeAttribute(attrIssue->attributeName());
            } else {
                if (newName != oldName)
                    facade->renameAttribute(oldName, newName);
                if (newValue != oldValue)
                    facade->setAttribute(newName, newValue);
            }
        }
    }
}
