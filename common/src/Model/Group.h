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

#ifndef __TrenchBroom__Group__
#define __TrenchBroom__Group__

#include "Model/Object.h"
#include "Model/ObjectSection.h"

namespace TrenchBroom {
    namespace Model {
        class Group : public Object, public ObjectSection {
        private:
            String m_name;
        public:
        private:
            void doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Object& object) const;
            bool doIntersects(const Object& object) const;
            
            void doAccept(ObjectVisitor& visitor);
            void doAccept(ObjectQuery& query) const;
            void doAcceptRecursively(ObjectVisitor& visitor);
            void doAcceptRecursively(ObjectQuery& visitor) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Group__) */
