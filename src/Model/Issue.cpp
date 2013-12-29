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

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Issue::Issue() :
        m_previous(NULL),
        m_next(NULL) {}
        
        Issue::~Issue() {}
        
        Issue* Issue::previous() const {
            return m_previous;
        }
        
        Issue* Issue::next() const {
            return m_next;
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
            m_previous->m_next = m_next;
            m_next->m_previous = m_previous;
            m_previous = NULL;
            m_next = NULL;
        }

        Issue* Issue::mergeWith(Issue* issue) {
            IssueGroup* group = new IssueGroup(this);
            return group->mergeWith(issue);
        }

        IssueGroup::IssueGroup(Issue* first) :
        m_first(first) {
            assert(m_first != NULL);
            m_first->replaceWith(this);
        }
        
        IssueGroup::~IssueGroup() {
            Issue* issue = m_first;
            while (issue != NULL) {
                Issue* next = issue->next();
                delete issue;
                issue = next;
            }
            m_first = NULL;
        }

        String IssueGroup::asString() const {
            StringStream str;
            Issue* issue = m_first;
            while (issue != NULL) {
                str << issue->asString();
                issue = issue->next();
                if (issue != NULL)
                    str << ", ";
            }
            return str.str();
        }

        Issue* IssueGroup::mergeWith(Issue* issue) {
            issue->insertBefore(m_first);
            m_first = issue;
            return this;
        }
    }
}
