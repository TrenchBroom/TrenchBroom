/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_FaceTypes_h
#define TrenchBroom_FaceTypes_h

#include <set>
#include <vector>
#include "Utility/Vec3.h"

namespace TrenchBroom {
    namespace Model {
        class Face;
        
        typedef std::vector<Face*> FaceList;
        static const FaceList EmptyFaceList;
        typedef std::set<Face*> FaceSet;
        static const FaceSet EmptyFaceSet;
        
        typedef VecMath::Vec3f FacePoints[3];
    }
}

#endif
