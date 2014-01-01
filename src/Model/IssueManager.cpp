/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "IssueManager.h"

#include "CollectionUtils.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueManager::IssueManager() :
        m_issueList(NULL) {}

        IssueManager::~IssueManager() {
            clearIssues();
            clearGenerators();
        }

        void IssueManager::registerGenerator(IssueGenerator* generator) {
            assert(!VectorUtils::contains(m_generators, generator));
            m_generators.push_back(generator);
        }

        size_t IssueManager::issueCount() const {
            return m_issueMap.size();
        }
        
        Issue* IssueManager::issues() const {
            return m_issueList;
        }

        void IssueManager::objectAdded(Object* object) {
            Issue* issue = findIssues(object);
            if (issue != NULL) {
                assert(m_issueMap.count(object) == 0);
                m_issueMap.insert(std::make_pair(object, issue));
                
                issue->insertBefore(m_issueList);
                m_issueList = issue;
                issueWasAddedNotifier(issue);
            }
        }
        
        void IssueManager::objectChanged(Object* object) {
            objectRemoved(object);
            objectAdded(object);
        }
        
        void IssueManager::objectRemoved(Object* object) {
            IssueMap::iterator it = m_issueMap.find(object);
            if (it != m_issueMap.end()) {
                Issue* issue = it->second;
                if (m_issueList == issue)
                    m_issueList = issue->next();
                issueWillBeRemovedNotifier(issue);
                issue->remove();
                delete issue;
                m_issueMap.erase(it);
            }
        }

        void IssueManager::setIgnoreIssue(Issue* issue, const bool ignore) {
            if (issue->ignore() != ignore) {
                issue->setIgnore(ignore);
                issueIgnoreChangedNotifier(issue);
            }
        }

        void IssueManager::clearIssues() {
            Issue* issue = m_issueList;
            while (issue != NULL) {
                Issue* next = issue->next();
                delete issue;
                issue = next;
            }
            m_issueList = NULL;
            m_issueMap.clear();
            issuesClearedNotifier();
        }
        
        void IssueManager::clearGenerators() {
            VectorUtils::clearAndDelete(m_generators);
        }
        
        Issue* IssueManager::findIssues(Object* object) {
            Issue* issue = NULL;
            GeneratorList::const_iterator it,end;
            for (it = m_generators.begin(), end = m_generators.end(); it != end; ++it) {
                const IssueGenerator* generator = *it;
                Issue* newIssue = generator->generate(object);
                if (newIssue != NULL) {
                    if (issue != NULL)
                        issue = issue->mergeWith(newIssue);
                    else
                        issue = newIssue;
                }
            }
            return issue;
        }
    }
}
