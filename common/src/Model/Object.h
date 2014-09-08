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

#ifndef __TrenchBroom__Object__
#define __TrenchBroom__Object__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Model/Issue.h"
#include "Model/ModelTypes.h"
#include "Model/Pickable.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class ObjectVisitor {
        public:
            virtual ~ObjectVisitor();
            void visit(Entity* entity);
            void visit(Brush* brush);
        private:
            virtual void doVisit(Entity* entity) = 0;
            virtual void doVisit(Brush* brush) = 0;
        };
        
        class ObjectQuery {
        public:
            virtual ~ObjectQuery();
            void query(const Entity* entity);
            void query(const Brush* brush);
        private:
            virtual void doQuery(const Entity* entity) = 0;
            virtual void doQuery(const Brush* brush) = 0;
        };
        
        class Object : public Pickable {
        private:
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            size_t m_childSelectionCount;
            IssueType m_hiddenIssues;
            
            Layer* m_layer;
        public:
            virtual ~Object();
            
            virtual const BBox3& bounds() const = 0;
            
            template <class T>
            static BBox3 bounds(const std::vector<T*>& objects) {
                if (objects.empty())
                    return BBox3();
                
                typedef std::vector<T*> List;
                typename List::const_iterator it = objects.begin();
                const typename List::const_iterator end = objects.end();
                
                BBox3 bounds = (*it)->bounds();
                while (++it != end)
                    bounds.mergeWith((*it)->bounds());
                return bounds;
            }
        public:
            size_t filePosition() const;
            void setFilePosition(size_t lineNumber, size_t lineCount);
            bool containsLine(size_t line) const;
            bool selected() const;
            virtual void select();
            virtual void deselect();
            bool partiallySelected() const;
            size_t childSelectionCount() const;
            void childSelectionChanged(bool newValue);
            
            IssueType hiddenIssues() const;
            void setHiddenIssues(IssueType hiddenIssues);
            bool isIssueHidden(const Issue* issue) const;
            void setIssueHidden(IssueType type, bool hidden);
            
            Layer* layer() const;
            void setLayer(Layer* layer);
            
            Object* clone(const BBox3& worldBounds) const;
            
            void transform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool contains(const Object& object) const;
            bool intersects(const Object& object) const;
            
            template <typename I, typename V>
            static void accept(I cur, I end, V& visitor) {
                while (cur != end) {
                    Object* object = *cur;
                    object->accept(visitor);
                    ++cur;
                }
            }
            
            template <typename I, typename V>
            static void acceptRecursively(I cur, I end, V& visitor) {
                while (cur != end) {
                    Object* object = *cur;
                    object->acceptRecursively(visitor);
                    ++cur;
                }
            }

            void accept(ObjectVisitor& visitor);
            void accept(ObjectQuery& query) const;
            void acceptRecursively(ObjectVisitor& visitor);
            void acceptRecursively(ObjectQuery& visitor) const;
        protected:
            Object();
            virtual Object* doClone(const BBox3& worldBounds) const = 0;

            void incChildSelectionCount();
            void decChildSelectionCount();
        private:
            virtual void doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) = 0;
            virtual bool doContains(const Object& object) const = 0;
            virtual bool doIntersects(const Object& object) const = 0;
            
            virtual void doAccept(ObjectVisitor& visitor) = 0;
            virtual void doAccept(ObjectQuery& query) const = 0;
            virtual void doAcceptRecursively(ObjectVisitor& visitor) = 0;
            virtual void doAcceptRecursively(ObjectQuery& visitor) const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Object__) */
