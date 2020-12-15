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

#include "RemoveEntityAttributesQuickFix.h"

#include "Model/Issue.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

namespace TrenchBroom {
    namespace Model {
        RemoveEntityAttributesQuickFix::RemoveEntityAttributesQuickFix(const IssueType issueType) :
        IssueQuickFix(issueType, "Delete properties") {}

        void RemoveEntityAttributesQuickFix::doApply(MapFacade* facade, const Issue* issue) const {
            const PushSelection push(facade);

            const EntityPropertyIssue* attrIssue = static_cast<const EntityPropertyIssue*>(issue);

            // If world node is affected, the selection will fail, but if nothing is selected,
            // the removeAttribute call will correctly affect worldspawn either way.

            facade->deselectAll();
            facade->select(issue->node());
            facade->removeAttribute(attrIssue->propertyKey());
        }
    }
}
