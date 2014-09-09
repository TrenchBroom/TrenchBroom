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

#include "Group.h"

namespace TrenchBroom {
    namespace Model {
        void Group::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
        }
        
        bool Group::doContains(const Object& object) const {
        }
        
        bool Group::doIntersects(const Object& object) const {
        }
        
        void Group::doAccept(ObjectVisitor& visitor) {
        }
        
        void Group::doAccept(ObjectQuery& query) const {
        }
        
        void Group::doAcceptRecursively(ObjectVisitor& visitor) {
        }
        
        void Group::doAcceptRecursively(ObjectQuery& visitor) const {
        }
    }
}
