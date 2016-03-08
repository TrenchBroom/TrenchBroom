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

#ifndef TrenchBroom_CollectMatchingIssuesVisitor
#define TrenchBroom_CollectMatchingIssuesVisitor

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
        class CollectMatchingIssuesVisitor : public NodeVisitor {
        private:
            const IssueGeneratorList& m_issueGenerators;
            P m_p;
            IssueList m_issues;
        public:
            CollectMatchingIssuesVisitor(const IssueGeneratorList& issueGenerators, const P& p = P()) :
            m_issueGenerators(issueGenerators),
            m_p(p) {}
            
            const IssueList& issues() const {
                return m_issues;
            }
        private:
            void doVisit(World* world)   { collectIssues(world);  }
            void doVisit(Layer* layer)   { collectIssues(layer);  }
            void doVisit(Group* group)   { collectIssues(group);  }
            void doVisit(Entity* entity) { collectIssues(entity); }
            void doVisit(Brush* brush)   { collectIssues(brush);  }
            
            void collectIssues(Node* node) {
                const IssueList& issues = node->issues(m_issueGenerators);
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

#endif /* defined(TrenchBroom_CollectMatchingIssuesVisitor) */
