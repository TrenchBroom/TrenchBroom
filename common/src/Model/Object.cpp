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

#include "Object.h"

namespace TrenchBroom {
    namespace Model {
        Object::Object() {}
        Object::~Object() {}
        
        const BBox3& Object::bounds() const {
            return doGetBounds();
        }

        void Object::pick(const Ray3& ray, PickResult& pickResult) const {
            doPick(ray, pickResult);
        }

        FloatType Object::intersectWithRay(const Ray3& ray) const {
            return doIntersectWithRay(ray);
        }

        Node* Object::container() const {
            return doGetContainer();
        }

        Layer* Object::layer() const {
            return doGetLayer();
        }
        
        Group* Object::group() const {
            return doGetGroup();
        }

        void Object::transform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {
            doTransform(transformation, lockTextures, worldBounds);
        }
        
        bool Object::contains(const Node* node) const {
            return doContains(node);
        }
        
        bool Object::intersects(const Node* node) const {
            return doIntersects(node);
        }
    }
}
