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

/*
 As an exception, this module is allowed to access modules from the View namespace.
 */
#include "View/ViewTypes.h"

#include <vector>


namespace TrenchBroom {
    namespace Model {
        class Issue;
        class IssueGroup;
        
        typedef size_t QuickFixType;
        typedef size_t IssueType;
        
        class QuickFix {
        public:
            typedef std::vector<QuickFix> List;
        private:
            QuickFixType m_fixType;
            IssueType m_issueType;
            String m_description;
        public:
            QuickFix(QuickFixType fixType, IssueType issueType, const String& description);
            
            bool operator==(const QuickFix& rhs) const;
            
            const String& description() const;
            void apply(Issue& issue, View::ControllerSPtr controller);
        };
        
        class Issue {
        private:
            IssueType m_type;
            Issue* m_previous;
            Issue* m_next;
            Issue* m_parent;
            QuickFix::List m_quickFixes;
        public:
            friend class IssueGroup;
            friend class QuickFix;
            
            virtual ~Issue();
            
            static IssueType freeType();

            virtual String description() const = 0;
            virtual void select(View::ControllerSPtr controller) = 0;
            const QuickFix::List& quickFixes() const;
            virtual void applyQuickFix(QuickFixType fixType, View::ControllerSPtr controller) = 0;
            
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
        protected:
            Issue(IssueType type);
            void addQuickFix(const QuickFix& quickFix);
        };

        class IssueGroup : public Issue {
        private:
            static const IssueType Type;
            Issue* m_first;
            size_t m_count;
        public:
            IssueGroup(Issue* first);
            ~IssueGroup();
            
            String description() const;
            void select(View::ControllerSPtr controller);
            void applyQuickFix(QuickFixType fixType, View::ControllerSPtr controller);

            size_t subIssueCount() const;
            Issue* subIssues() const;
            
            Issue* mergeWith(Issue* issue);
        };
        
    }
}


#endif /* defined(__TrenchBroom__Issue__) */
