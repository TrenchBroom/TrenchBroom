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

#ifndef TrenchBroom_CollectMatchingIssuesVisitor
#define TrenchBroom_CollectMatchingIssuesVisitor

#include "Model/NodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename P>
        class CollectMatchingIssuesVisitor : public NodeVisitor {
        private:
            const std::vector<IssueGenerator*>& m_issueGenerators;
            P m_p;
            std::vector<Issue*> m_issues;
        public:
            CollectMatchingIssuesVisitor(const std::vector<IssueGenerator*>& issueGenerators, const P& p = P()) :
            m_issueGenerators(issueGenerators),
            m_p(p) {}

            const std::vector<Issue*>& issues() const {
                return m_issues;
            }
        private:
            void doVisit(World* world)   override { collectIssues(world);  }
            void doVisit(Layer* layer)   override { collectIssues(layer);  }
            void doVisit(Group* group)   override { collectIssues(group);  }
            void doVisit(Entity* entity) override { collectIssues(entity); }
            void doVisit(BrushNode* brush)   override { collectIssues(brush);  }

            void collectIssues(Node* node) {
                for (Issue* issue : node->issues(m_issueGenerators)) {
                    if (m_p(issue))
                        m_issues.push_back(issue);
                }
            }
        };
    }
}

#endif /* defined(TrenchBroom_CollectMatchingIssuesVisitor) */
