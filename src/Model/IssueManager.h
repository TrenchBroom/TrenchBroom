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

#ifndef __TrenchBroom__IssueManager__
#define __TrenchBroom__IssueManager__

#include "Model/ModelTypes.h"
#include "Notifier.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Issue;
        class IssueGenerator;
        
        class IssueManager {
        private:
            typedef std::map<Object*, Issue*> IssueMap;
            typedef std::vector<IssueGenerator*> GeneratorList;
            
            GeneratorList m_generators;
            Issue* m_issueList;
            IssueMap m_issueMap;
        public:
            Notifier1<Issue*> issueWasAddedNotifier;
            Notifier1<Issue*> issueWillBeRemovedNotifier;
            Notifier1<Issue*> issueIgnoreChangedNotifier;
            Notifier0 issuesClearedNotifier;
        public:
            IssueManager();
            ~IssueManager();
            
            void registerGenerator(IssueGenerator* generator);
            
            size_t issueCount() const;
            Issue* issues() const;
            
            void objectAdded(Object* object);
            void objectChanged(Object* object);
            void objectRemoved(Object* object);
            void setIgnoreIssue(Issue* issue, bool ignore);
            
            void clearIssues();
            void clearGenerators();
        private:
            
            Issue* findIssues(Object* object);
        };
    }
}

#endif /* defined(__TrenchBroom__IssueManager__) */
