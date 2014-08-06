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
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"
#include "Model/Object.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueManager::IssueManager() :
        m_defaultHiddenGenerators(0),
        m_hiddenGenerators(m_defaultHiddenGenerators) {}

        IssueManager::~IssueManager() {
            clearIssues();
            clearGenerators();
        }

        void IssueManager::registerGenerator(IssueGenerator* generator, const bool showByDefault) {
            assert(!VectorUtils::contains(m_generators, generator));
            m_generators.push_back(generator);
            if (!showByDefault) {
                m_defaultHiddenGenerators |= generator->type();
                m_hiddenGenerators |= generator->type();
            }
        }

        const IssueManager::GeneratorList& IssueManager::registeredGenerators() const {
            return m_generators;
        }

        int IssueManager::hiddenGenerators() const {
            return m_hiddenGenerators;
        }

        void IssueManager::setHiddenGenerators(const int value) {
            m_hiddenGenerators = value;
        }

        void IssueManager::resetHiddenGenerators() {
            m_hiddenGenerators = m_defaultHiddenGenerators;
        }

        size_t IssueManager::issueCount() const {
            return m_issues.size();
        }
        
        const IssueList& IssueManager::issues() const {
            return m_issues;
        }

        void IssueManager::setIssueHidden(Issue* issue, const bool hidden) {
            if (issue->isHidden() != hidden) {
                issue->setHidden(hidden);
                issueIgnoreChangedNotifier(issue);
            }
        }

        void IssueManager::clearIssues() {
            VectorUtils::clearAndDelete(m_issues);
        }
        
        void IssueManager::clearGenerators() {
            VectorUtils::clearAndDelete(m_generators);
            m_defaultHiddenGenerators = 0;
            m_hiddenGenerators = m_defaultHiddenGenerators;
        }
        
        IssueList IssueManager::findIssues(Object* object) {
            IssueList result;
            GeneratorList::const_iterator it,end;
            for (it = m_generators.begin(), end = m_generators.end(); it != end; ++it) {
                const IssueGenerator* generator = *it;
                generator->generate(object, result);
            }
            return result;
        }

        void IssueManager::insertIssues(Object* object, const IssueList& issues) {
            const IssueQuery query(object);
            IssueCmp cmp;
            IssueList::iterator it = std::lower_bound(m_issues.begin(), m_issues.end(), query, cmp);
            m_issues.insert(it, issues.begin(), issues.end());
        }
        
        void IssueManager::removeIssues(Object* object) {
            const IssueQuery query(object);
            IssueCmp cmp;
            IssueList::iterator begin = std::lower_bound(m_issues.begin(), m_issues.end(), query, cmp);
            IssueList::iterator end   = std::upper_bound(m_issues.begin(), m_issues.end(), query, cmp);
            m_issues.erase(begin, end);
        }
    }
}
