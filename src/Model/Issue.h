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

#ifndef __TrenchBroom__Issue__
#define __TrenchBroom__Issue__

#include "StringUtils.h"

namespace TrenchBroom {
    namespace Model {
        class IssueGroup;
        
        class Issue {
        private:
            Issue* m_previous;
            Issue* m_next;
            Issue* m_parent;
        public:
            friend class IssueGroup;
            
            Issue();
            virtual ~Issue();
            
            virtual String asString() const = 0;
            
            virtual size_t subIssueCount() const;
            virtual Issue* subIssues() const;
            
            Issue* previous() const;
            Issue* next() const;
            Issue* parent() const;

            void insertAfter(Issue* previous);
            void insertBefore(Issue* next);
            void replaceWith(Issue* issue);
            void remove();
            
            virtual Issue* mergeWith(Issue* issue);
        };

        class IssueGroup : public Issue {
        private:
            Issue* m_first;
            size_t m_count;
        public:
            IssueGroup(Issue* first);
            ~IssueGroup();
            
            String asString() const;

            size_t subIssueCount() const;
            Issue* subIssues() const;
            
            Issue* mergeWith(Issue* issue);
        };
        
    }
}


#endif /* defined(__TrenchBroom__Issue__) */
