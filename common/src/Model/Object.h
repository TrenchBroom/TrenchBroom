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

#ifndef TrenchBroom_Object
#define TrenchBroom_Object

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "Model/Pickable.h"

namespace TrenchBroom {
    namespace Model {
        class NodeSnapshot;
        class PickResult;
        
        class Object {
        protected:
            Object();
        public:
            virtual ~Object();
            
            Node* container() const;
            Layer* layer() const;
            Group* group() const;
        
            void transform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool contains(const Node* object) const;
            bool intersects(const Node* object) const;
        private: // subclassing interface
            virtual Node* doGetContainer() const = 0;
            virtual Layer* doGetLayer() const = 0;
            virtual Group* doGetGroup() const = 0;

            virtual void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) = 0;
            virtual bool doContains(const Node* node) const = 0;
            virtual bool doIntersects(const Node* node) const = 0;
        };
    }
}


#endif /* defined(TrenchBroom_Object) */
