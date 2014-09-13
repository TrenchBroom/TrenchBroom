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

#include "Notifier.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class IssueQuery;
        class IssueGenerator;
        
        class IssueManager {
        public:
            typedef std::vector<IssueGenerator*> GeneratorList;
        private:
            struct IssueCmp {
                bool operator()(const Issue* issue1, const Issue* issue2) const;
                bool operator()(const Issue* issue, const IssueQuery& query) const;
                bool operator()(const IssueQuery& query, const Issue* issue) const;
            };
            
            GeneratorList m_generators;
            IssueList m_issues;
            
            int m_defaultHiddenGenerators;
        public:
            Notifier0 issuesDidChangeNotifier;
            Notifier1<Issue*> issueIgnoreChangedNotifier;
        public:
            IssueManager();
            ~IssueManager();
            
            void registerGenerator(IssueGenerator* generator, bool showByDefault);
            const GeneratorList& registeredGenerators() const;
            
            int defaultHiddenGenerators() const;
            
            size_t issueCount() const;
            const IssueList& issues() const;
            
            template <typename I>
            void addObjects(I cur, I end) {
                while (cur != end) {
                    addObject(*cur);
                    ++cur;
                }
                issuesDidChangeNotifier();
            }
            
            template <typename I>
            void removeObjects(I cur, I end) {
                while (cur != end) {
                    removeObject(*cur);
                    ++cur;
                }
                issuesDidChangeNotifier();
            }
            
            template <typename I>
            void updateObjects(I cur, I end) {
                while (cur != end) {
                    updateObject(*cur);
                    ++cur;
                }
                issuesDidChangeNotifier();
            }
            
            void addObject(Object* object);
            void removeObject(Object* object);
            void updateObject(Object* object);
            
            void setIssueHidden(Issue* issue, bool hidden);
            
            void clearIssues();
            void clearGenerators();
        private:
            IssueList findIssues(Object* object);
            void insertIssues(Object* object, const IssueList& issues);
            void removeIssues(Object* object);
        };
    }
}

#endif /* defined(__TrenchBroom__IssueManager__) */
