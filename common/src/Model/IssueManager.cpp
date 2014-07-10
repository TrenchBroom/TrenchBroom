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

#include "IssueManager.h"

#include "CollectionUtils.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueManager::IssuePair::IssuePair() :
        first(NULL),
        last(NULL) {}
        
        IssueManager::IssuePair::IssuePair(Issue* i_first, Issue* i_last) :
        first(i_first),
        last(i_last) {
            assert(first != NULL);
            assert(last != NULL);
        }
        
        void IssueManager::IssuePair::preprend(Issue* issue) {
            issue->insertBefore(first);
            first = issue;
        }
        
        void IssueManager::IssuePair::append(Issue* issue) {
            issue->insertAfter(last);
            last = issue;
        }

        IssueManager::IssueManager() :
        m_issueList(NULL),
        m_defaultHiddenGenerators(0) {}

        IssueManager::~IssueManager() {
            clearIssues();
            clearGenerators();
        }

        void IssueManager::registerGenerator(IssueGenerator* generator, const bool showByDefault) {
            assert(!VectorUtils::contains(m_generators, generator));
            m_generators.push_back(generator);
            if (!showByDefault)
                m_defaultHiddenGenerators |= generator->type();
        }

        const IssueManager::GeneratorList& IssueManager::registeredGenerators() const {
            return m_generators;
        }

        int IssueManager::defaultHiddenGenerators() const {
            return m_defaultHiddenGenerators;
        }

        size_t IssueManager::issueCount() const {
            return m_issueMap.size();
        }
        
        Issue* IssueManager::issues() const {
            return m_issueList;
        }

        void IssueManager::objectAdded(Object* object) {
            Issue* first = findIssues(object);
            if (first != NULL) {
                Issue* last = first;
                while (last->next() != NULL)
                    last = last->next();
                
                assert(m_issueMap.count(object) == 0);
                m_issueMap.insert(std::make_pair(object, IssuePair(first, last)));
                
                first->insertBefore(m_issueList);
                m_issueList = first;

                while (first != last) {
                    issueWasAddedNotifier(first);
                    first = first->next();
                }
                issueWasAddedNotifier(last);
            }
        }
        
        void IssueManager::objectChanged(Object* object) {
            objectRemoved(object);
            objectAdded(object);
        }
        
        void IssueManager::objectRemoved(Object* object) {
            IssueMap::iterator it = m_issueMap.find(object);
            if (it != m_issueMap.end()) {
                Issue* first = it->second.first;
                Issue* last = it->second.last;
                
                if (m_issueList == first)
                    m_issueList = last->next();

                first->remove(last);
                while (first != NULL) {
                    issueWillBeRemovedNotifier(first);
                    Issue* tmp = first;
                    first = first->next();
                    delete tmp;
                }
                m_issueMap.erase(it);
            }
        }

        void IssueManager::setIssueHidden(Issue* issue, const bool hidden) {
            if (issue->isHidden() != hidden) {
                issue->setHidden(hidden);
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
            m_defaultHiddenGenerators = 0;
        }
        
        Issue* IssueManager::findIssues(Object* object) {
            Issue* issue = NULL;
            GeneratorList::const_iterator it,end;
            for (it = m_generators.begin(), end = m_generators.end(); it != end; ++it) {
                const IssueGenerator* generator = *it;
                Issue* newIssue = generator->generate(object);
                if (newIssue != NULL) {
                    if (issue != NULL)
                        newIssue->insertAfter(issue);
                    else
                        issue = newIssue;
               }
            }
            return issue;
        }
    }
}
