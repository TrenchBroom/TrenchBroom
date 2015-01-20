/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__CollectMatchingIssuesVisitor__
#define __TrenchBroom__CollectMatchingIssuesVisitor__

#include "Model/NodeVisitor.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/ModelTypes.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        template <typename P>
        class CollectMatchingIssuesVisitor : public ConstNodeVisitor {
        private:
            P m_p;
            IssueList m_issues;
        public:
            CollectMatchingIssuesVisitor(const P& p = P()) : m_p(p) {}
            
            const IssueList& issues() const {
                return m_issues;
            }
        private:
            void doVisit(const World* world)   { collectIssues(world);  }
            void doVisit(const Layer* layer)   { collectIssues(layer);  }
            void doVisit(const Group* group)   { collectIssues(group);  }
            void doVisit(const Entity* entity) { collectIssues(entity); }
            void doVisit(const Brush* brush)   { collectIssues(brush);  }
            
            void collectIssues(const Node* node) {
                const IssueList& issues = node->issues();
                IssueList::const_iterator it, end;
                for (it = issues.begin(), end = issues.end(); it != end; ++it) {
                    Issue* issue = *it;
                    if (m_p(issue))
                        m_issues.push_back(issue);
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__CollectMatchingIssuesVisitor__) */
