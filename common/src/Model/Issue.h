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
        class Object;
        
        typedef int IssueType;
        
        class IssueQuery {
        private:
            Object* m_object;
        public:
            IssueQuery(Object* object);
            int compare(const Issue* issue) const;
        };
        
        class Issue {
        private:
            size_t m_seqIndex;
            IssueType m_type;
            QuickFix::List m_quickFixes;
            QuickFix::List m_deletableFixes;
        public:
            friend class QuickFix;
            virtual ~Issue();
            static IssueType freeType();

            int compare(const Issue* issue) const;
            int compare(const Object* object) const;

            size_t seqIndex() const;
            IssueType type() const;
            bool hasType(IssueType mask) const;
            
            virtual size_t filePosition() const = 0;
            virtual String description() const = 0;
            virtual void select(View::ControllerSPtr controller) = 0;
            const QuickFix::List& quickFixes() const;
            virtual void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller);

            bool isHidden() const;
            void setHidden(bool hidden);
        protected:
            Issue(IssueType type);
            void addSharedQuickFix(const QuickFix& quickFix);
            void addDeletableQuickFix(const QuickFix* quickFix);
        private:
            virtual bool doIsHidden(IssueType type) const = 0;
            virtual void doSetHidden(IssueType type, bool hidden) = 0;
            virtual int doCompare(const Issue* issue) const = 0;
            virtual int doCompare(const Object* object) const = 0;
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
            int doCompare(const Issue* issue) const;
            int doCompare(const Object* object) const;
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
            int doCompare(const Issue* issue) const;
            int doCompare(const Object* object) const;
        };
    }
}


#endif /* defined(__TrenchBroom__Issue__) */
