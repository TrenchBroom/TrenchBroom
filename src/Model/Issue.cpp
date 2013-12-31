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

#include "Issue.h"

#include "CollectionUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        QuickFix::QuickFix(const QuickFixType fixType, const IssueType issueType, const String& description) :
        m_fixType(fixType),
        m_issueType(issueType),
        m_description(description) {}
        
        bool QuickFix::operator==(const QuickFix& rhs) const {
            return m_fixType == rhs.m_fixType && m_issueType == rhs.m_issueType;
        }

        const String& QuickFix::description() const {
            return m_description;
        }
        
        void QuickFix::apply(Issue& issue, View::ControllerSPtr controller) {
            assert(issue.m_type == m_issueType);
            issue.applyQuickFix(m_fixType, controller);
        }

        Issue::~Issue() {}
        
        IssueType Issue::freeType() {
            static IssueType type = 0;
            return type++;
        }

        const QuickFix::List& Issue::quickFixes() const {
            return m_quickFixes;
        }

        size_t Issue::subIssueCount() const {
            return 0;
        }
        
        Issue* Issue::subIssues() const {
            return NULL;
        }
        
        Issue* Issue::previous() const {
            return m_previous;
        }
        
        Issue* Issue::next() const {
            return m_next;
        }

        Issue* Issue::parent() const {
            return m_parent;
        }

        void Issue::insertAfter(Issue* previous) {
            if (previous != NULL) {
                m_previous = previous;
                m_next = previous->m_next;
                m_previous->m_next = this;
                if (m_next != NULL)
                    m_next->m_previous = this;
            }
        }
        
        void Issue::insertBefore(Issue* next) {
            if (next != NULL) {
                m_previous = next->m_previous;
                m_next = next;
                if (m_previous != NULL)
                    m_previous->m_next = this;
                m_next->m_previous = this;
            }
        }
        
        void Issue::replaceWith(Issue* issue) {
            if (m_previous != NULL)
                m_previous->m_next = issue;
            if (m_next != NULL)
                m_next->m_previous = issue;
            issue->m_previous = m_previous;
            issue->m_next = m_next;
            m_previous = NULL;
            m_next = NULL;
        }

        void Issue::remove() {
            if (m_previous != NULL)
                m_previous->m_next = m_next;
            if (m_next != NULL)
                m_next->m_previous = m_previous;
            m_previous = NULL;
            m_next = NULL;
        }

        Issue* Issue::mergeWith(Issue* issue) {
            IssueGroup* group = new IssueGroup(this);
            return group->mergeWith(issue);
        }

        Issue::Issue(const IssueType type) :
        m_type(type),
        m_previous(NULL),
        m_next(NULL),
        m_parent(NULL) {}
        
        void Issue::addQuickFix(const QuickFix& quickFix) {
            m_quickFixes.push_back(quickFix);
        }

        void Issue::addQuickFixes(const QuickFix::List& quickFixes) {
            VectorUtils::append(m_quickFixes, quickFixes);
        }

        const IssueType IssueGroup::Type = Issue::freeType();
        
        IssueGroup::IssueGroup(Issue* first) :
        Issue(Type),
        m_first(first),
        m_last(first),
        m_count(1) {
            assert(m_first != NULL);
            m_first->replaceWith(this);
            m_first->m_parent = this;
            addQuickFixes(m_first->quickFixes());
        }
        
        IssueGroup::~IssueGroup() {
            Issue* issue = m_first;
            while (issue != NULL) {
                Issue* next = issue->next();
                delete issue;
                issue = next;
            }
            m_first = NULL;
            m_last = NULL;
        }

        String IssueGroup::description() const {
            StringStream str;
            Issue* issue = m_first;
            while (issue != NULL) {
                str << issue->description();
                issue = issue->next();
                if (issue != NULL)
                    str << ", ";
            }
            return str.str();
        }

        void IssueGroup::select(View::ControllerSPtr controller) {
            m_first->select(controller);
        }

        void IssueGroup::applyQuickFix(const QuickFixType fixType, View::ControllerSPtr controller) {
            Issue* issue = m_first;
            while (issue != NULL) {
                issue->applyQuickFix(fixType, controller);
                issue = issue->next();
            }
        }

        size_t IssueGroup::subIssueCount() const {
            return m_count;
        }
        
        Issue* IssueGroup::subIssues() const {
            return m_first;
        }

        Issue* IssueGroup::mergeWith(Issue* issue) {
            if (issue->subIssueCount() == 0) {
                issue->insertAfter(m_last);
                m_last = issue;
                m_last->m_parent = this;
                addQuickFixes(m_last->quickFixes());
                ++m_count;
            } else {
                Issue* subIssue = issue->subIssues();
                while (subIssue != NULL) {
                    subIssue->insertAfter(m_last);
                    m_last = subIssue;
                    m_last->m_parent = this;
                    addQuickFixes(m_last->quickFixes());
                    ++m_count;
                    subIssue = subIssue->next();
                }
                delete issue;
            }
            
            return this;
        }
    }
}
