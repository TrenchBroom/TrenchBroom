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
            
            Type type() const;
            void setFilePosition(const size_t lineNumber, const size_t lineCount);
            bool selected() const;
            virtual bool select();
            virtual bool deselect();
            bool partiallySelected() const;
            size_t childSelectionCount() const;
            void incChildSelectionCount();
            void decChildSelectionCount();
            
            Object* clone(const BBox3& worldBounds) const;
            void transform(const Mat4x4& transformation, const bool lockTextures, const bool invertFaceOrientation, const BBox3& worldBounds);
        protected:
            Object(const Type type);
            virtual Object* doClone(const BBox3& worldBounds) const = 0;
        private:
            virtual void doTransform(const Mat4x4& transformation, const bool lockTextures, const bool invertFaceOrientation, const BBox3& worldBounds) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Object__) */
