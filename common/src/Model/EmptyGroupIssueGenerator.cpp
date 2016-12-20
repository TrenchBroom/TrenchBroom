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

#include "EmptyGroupIssueGenerator.h"

#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Group.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class EmptyGroupIssueGenerator::EmptyGroupIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            EmptyGroupIssue(Group* group) :
            Issue(group) {}
        private:
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                const Group* group = static_cast<Group*>(node());
                return "Group '" + group->name() + "' does not contain any objects";
            }
        };
        
        const IssueType EmptyGroupIssueGenerator::EmptyGroupIssue::Type = Issue::freeType();
        
        class EmptyGroupIssueGenerator::EmptyGroupIssueQuickFix : public IssueQuickFix {
        public:
            EmptyGroupIssueQuickFix() :
            IssueQuickFix(EmptyGroupIssue::Type, "Delete groups") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->deleteObjects();
            }
        };
        
        EmptyGroupIssueGenerator::EmptyGroupIssueGenerator() :
        IssueGenerator(EmptyGroupIssue::Type, "Empty group") {
            addQuickFix(new EmptyGroupIssueQuickFix());
        }
        
        void EmptyGroupIssueGenerator::doGenerate(Group* group, IssueList& issues) const {
            ensure(group != NULL, "group is null");
            if (!group->hasChildren())
                issues.push_back(new EmptyGroupIssue(group));
        }
    }
}
