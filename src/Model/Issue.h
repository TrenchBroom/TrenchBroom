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

#ifndef __TrenchBroom__Issue__
#define __TrenchBroom__Issue__

#include "StringUtils.h"

#include "Model/QuickFix.h"

/*
 As an exception, this module is allowed to access modules from the View namespace.
 */
#include "View/ViewTypes.h"

#include <vector>


namespace TrenchBroom {
    namespace Model {
        class Issue;
        class IssueGroup;
        
        typedef size_t IssueType;
        
        class Issue {
        private:
            IssueType m_type;
            Issue* m_previous;
            Issue* m_next;
            QuickFix::List m_quickFixes;
            QuickFix::List m_deletableFixes;
        public:
            friend class IssueGroup;
            friend class QuickFix;
            
            virtual ~Issue();
            
            static IssueType freeType();

            IssueType type() const;
            bool hasType(IssueType mask) const;
            
            virtual size_t filePosition() const = 0;
            virtual String description() const = 0;
            virtual void select(View::ControllerSPtr controller) = 0;
            const QuickFix::List& quickFixes() const;
            virtual void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller);

            bool isHidden() const;
            void setHidden(bool hidden);
            
            Issue* previous() const;
            Issue* next() const;
            Issue* parent() const;

            void insertAfter(Issue* previous);
            void insertBefore(Issue* next);
            void remove(Issue* last = NULL);
        protected:
            Issue(IssueType type);
            void addSharedQuickFix(const QuickFix& quickFix);
            void addDeletableQuickFix(const QuickFix* quickFix);
            virtual bool doIsHidden(IssueType type) const = 0;
            virtual void doSetHidden(IssueType type, bool hidden) = 0;
        };

        class Entity;
        class EntityIssue : public Issue {
        private:
            Entity* m_entity;
        public:
            size_t filePosition() const;
            void select(View::ControllerSPtr controller);
        protected:
            EntityIssue(IssueType type, Entity* entity);
            Entity* entity() const;
        private:
            bool doIsHidden(IssueType type) const;
            void doSetHidden(IssueType type, bool hidden);
        };

        class Brush;
        class BrushIssue : public Issue {
        private:
            Brush* m_brush;
        public:
            size_t filePosition() const;
            void select(View::ControllerSPtr controller);
        protected:
            BrushIssue(IssueType type, Brush* brush);
            Brush* brush() const;
        private:
            bool doIsHidden(IssueType type) const;
            void doSetHidden(IssueType type, bool hidden);
        };
    }
}


#endif /* defined(__TrenchBroom__Issue__) */
