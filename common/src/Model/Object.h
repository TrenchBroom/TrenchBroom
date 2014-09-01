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
            virtual void doVisit(Entity* entity);
            virtual void doVisit(Brush* entity);
        };
        
        class Object : public Pickable {
        public:
            typedef enum {
                Type_Entity,
                Type_Brush
            } Type;
        private:
            Type m_type;
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
            
            Type type() const;
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
            
            virtual Layer* layer() const;
            virtual void setLayer(Layer* layer);
            
            Object* clone(const BBox3& worldBounds) const;
            void transform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            
            bool contains(const Object& object) const;
            bool contains(const Entity& entity) const;
            bool contains(const Brush& brush) const;
            bool containedBy(const Object& object) const;
            bool containedBy(const Entity& entity) const;
            bool containedBy(const Brush& brush) const;
            bool intersects(const Object& object) const;
            bool intersects(const Entity& entity) const;
            bool intersects(const Brush& brush) const;
            void visit(ObjectVisitor& visitor);
        protected:
            Object(const Type type);
            virtual Object* doClone(const BBox3& worldBounds) const = 0;

            void incChildSelectionCount();
            void decChildSelectionCount();
        private:
            virtual void doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) = 0;
            virtual bool doContains(const Object& object) const = 0;
            virtual bool doContains(const Entity& entity) const = 0;
            virtual bool doContains(const Brush& brush) const = 0;
            virtual bool doContainedBy(const Object& object) const = 0;
            virtual bool doContainedBy(const Entity& entity) const = 0;
            virtual bool doContainedBy(const Brush& brush) const = 0;
            virtual bool doIntersects(const Object& object) const = 0;
            virtual bool doIntersects(const Entity& entity) const = 0;
            virtual bool doIntersects(const Brush& brush) const = 0;
            virtual void doVisit(ObjectVisitor& visitor) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Object__) */
