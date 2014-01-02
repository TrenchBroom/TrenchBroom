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
            static size_t index = 0;
            assert(index < sizeof(IssueType) * 8);
            return 1 << index++;
        }

        bool Issue::hasType(IssueType mask) const {
            return m_type & mask;
        }

        const QuickFix::List& Issue::quickFixes() const {
            return m_quickFixes;
        }

        bool Issue::ignore() const {
            return doGetIgnore(m_type);
        }

        void Issue::setIgnore(const bool ignore) {
            doSetIgnore(m_type, ignore);
        }

        Issue* Issue::previous() const {
            return m_previous;
        }
        
        Issue* Issue::next() const {
            return m_next;
        }

        void Issue::insertAfter(Issue* previous) {
            if (previous != NULL) {
                Issue* next = previous->m_next;
                
                Issue* lastSuccessor = this;
                while (lastSuccessor->next() != NULL)
                    lastSuccessor = lastSuccessor->next();
                
                m_previous = previous;
                m_previous->m_next = this;
                
                lastSuccessor->m_next = next;
                if (next != NULL)
                    next->m_previous = lastSuccessor;
            }
        }
        
        void Issue::insertBefore(Issue* next) {
            if (next != NULL) {
                Issue* previous = next->m_previous;
                
                Issue* lastSuccessor = this;
                while (lastSuccessor->next() != NULL)
                    lastSuccessor = lastSuccessor->next();
                
                if (previous != NULL)
                    previous->m_next = this;
                m_previous = previous;
                
                next->m_previous = lastSuccessor;
                lastSuccessor->m_next = next;
            }
        }
        
        void Issue::remove(Issue* last) {
            if (last == NULL)
                last = this;
            
            if (m_previous != NULL)
                m_previous->m_next = last->m_next;
            if (last->m_next != NULL)
                last->m_next->m_previous = m_previous;
            m_previous = NULL;
            last->m_next = NULL;
        }

        Issue::Issue(const IssueType type) :
        m_type(type),
        m_previous(NULL),
        m_next(NULL) {}
        
        void Issue::addQuickFix(const QuickFix& quickFix) {
            m_quickFixes.push_back(quickFix);
        }

        void Issue::addQuickFixes(const QuickFix::List& quickFixes) {
            VectorUtils::append(m_quickFixes, quickFixes);
        }
    }
}
