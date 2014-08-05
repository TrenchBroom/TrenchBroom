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

#ifndef __TrenchBroom__IssueManager__
#define __TrenchBroom__IssueManager__

#include "Model/ModelTypes.h"
#include "Notifier.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/Object.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Issue;
        class IssueGenerator;
        
        class IssueManager {
        public:
            typedef std::vector<IssueGenerator*> GeneratorList;
        private:
            struct IssuePair {
                Issue* first;
                Issue* last;
                
                IssuePair();
                IssuePair(Issue* i_first, Issue* i_last);
                
                void preprend(Issue* issue);
                void append(Issue* issue);
            };
            
            typedef std::map<Object*, IssuePair> IssueMap;
            
            GeneratorList m_generators;
            
            Issue* m_issueList;
            IssueMap m_issueMap;
            int m_defaultHiddenGenerators;
        public:
            Notifier2<Issue*, Issue*> issuesWereAddedNotifier;
            Notifier2<Issue*, Issue*> issuesWillBeRemovedNotifier;
            Notifier1<Issue*> issueIgnoreChangedNotifier;
            Notifier0 issuesClearedNotifier;
        public:
            IssueManager();
            ~IssueManager();
            
            void registerGenerator(IssueGenerator* generator, bool showByDefault);
            const GeneratorList& registeredGenerators() const;
            int defaultHiddenGenerators() const;
            
            size_t issueCount() const;
            Issue* issues() const;
            
            template <typename I>
            void addObjects(I cur, I end) {
                if (cur == end)
                    return;
                
                Issue* firstIssue = NULL;
                Issue* lastIssue = NULL;

                while (cur != end) {
                    Object* object = *cur;
                    Issue* currentFirst = findIssues(object);
                    if (currentFirst != NULL) {
                        Issue* currentLast = Issue::lastIssue(currentFirst);
                        if (firstIssue == NULL)
                            firstIssue = currentFirst;
                        else
                            currentFirst->insertAfter(lastIssue);
                        lastIssue = currentLast;
                        
                        assert(m_issueMap.count(object) == 0);
                        m_issueMap.insert(std::make_pair(object, IssuePair(currentFirst, currentLast)));
                    }
                    ++cur;
                }
                
                if (firstIssue != NULL && lastIssue != NULL) {
                    lastIssue->insertBefore(m_issueList);
                    m_issueList = firstIssue;
                    issuesWereAddedNotifier(firstIssue, lastIssue);
                }
            }
            
            template <typename I>
            void removeObjects(I cur, I end) {
                if (cur == end)
                    return;
                
                while (cur != end) {
                    Object* object = *cur;
                    IssueMap::iterator issIt = m_issueMap.find(object);
                    if (issIt != m_issueMap.end()) {
                        Issue* first = issIt->second.first;
                        Issue* last  = issIt->second.last;
                        
                        issuesWillBeRemovedNotifier(first, last);
                        
                        if (m_issueList == first)
                            m_issueList = last->next();
                        first->remove(last);
                        while (first != NULL) {
                            Issue* tmp = first;
                            first = first->next();
                            delete tmp;
                        }
                        
                        m_issueMap.erase(issIt);
                    }
                    ++cur;
                }
            }
            
            template <typename I>
            void updateObjects(I cur, I end) {
                removeObjects(cur, end);
                addObjects(cur, end);
            }
            
            void setIssueHidden(Issue* issue, bool hidden);
            
            void clearIssues();
            void clearGenerators();
        private:
            Issue* findIssues(Object* object);
        };
    }
}

#endif /* defined(__TrenchBroom__IssueManager__) */
