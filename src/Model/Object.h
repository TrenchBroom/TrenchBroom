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

#ifndef __TrenchBroom__Object__
#define __TrenchBroom__Object__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "Model/Pickable.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Object : public Pickable {
        public:
            typedef enum {
                OTEntity,
                OTBrush
            } Type;
        private:
            Type m_type;
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            size_t m_childSelectionCount;
        public:
            virtual ~Object();
            
            static BBox3 bounds(const ObjectList& objects);
            
            Type type() const;
            void setFilePosition(const size_t lineNumber, const size_t lineCount);
            bool selected() const;
            bool select();
            bool deselect();
            bool partiallySelected() const;
            size_t childSelectionCount() const;
            void incChildSelectionCount();
            void decChildSelectionCount();
            
            Object* clone(const BBox3& worldBounds) const;
            void transform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds);
            
            bool contains(const Object& object) const;
            bool contains(const Entity& entity) const;
            bool contains(const Brush& brush) const;
            bool containedBy(const Object& object) const;
            bool containedBy(const Entity& entity) const;
            bool containedBy(const Brush& brush) const;
            bool intersects(const Object& object) const;
            bool intersects(const Entity& entity) const;
            bool intersects(const Brush& brush) const;
        protected:
            Object(const Type type);
            virtual Object* doClone(const BBox3& worldBounds) const = 0;
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
        };
    }
}

#endif /* defined(__TrenchBroom__Object__) */
