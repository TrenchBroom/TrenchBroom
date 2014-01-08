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

#ifndef TrenchBroom_FaceEdgesIterator_h
#define TrenchBroom_FaceEdgesIterator_h

#include "NestedIterator.h"
#include "Model/ModelTypes.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        struct FaceEdgesIterator {
            typedef BrushEdgeList::const_iterator InnerIterator;
            typedef NestedIterator<BrushFaceList::const_iterator, FaceEdgesIterator> OuterIterator;
            
            static bool isInnerEmpty(BrushFaceList::const_iterator it) {
                BrushFace* face = *it;
                return face->edges().empty();
            }

            static OuterIterator begin(const BrushFaceList& faces) {
                return OuterIterator(faces.begin(), faces.end());
            }
            
            static OuterIterator end(const BrushFaceList& faces) {
                return OuterIterator(faces.end());
            }
            
            static InnerIterator beginInner(BrushFaceList::const_iterator it) {
                BrushFace* face = *it;
                return face->edges().begin();
            }
            
            static InnerIterator endInner(BrushFaceList::const_iterator it) {
                BrushFace* face = *it;
                return face->edges().end();
            }
        };
    }
}


#endif
