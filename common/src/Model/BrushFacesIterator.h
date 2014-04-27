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

#ifndef TrenchBroom_BrushFacesIterator_h
#define TrenchBroom_BrushFacesIterator_h

#include "NestedIterator.h"
#include "Model/ModelTypes.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        struct BrushFacesIterator {
            typedef BrushFaceList::const_iterator InnerIterator;
            typedef NestedIterator<BrushList::const_iterator, BrushFacesIterator> OuterIterator;
            
            static bool isInnerEmpty(BrushList::const_iterator it) {
                Brush* brush = *it;
                return brush->faces().empty();
            }
            
            static OuterIterator begin(const BrushList& brushes) {
                return OuterIterator(brushes.begin(), brushes.end());
            }
            
            static OuterIterator end(const BrushList& brushes) {
                return OuterIterator(brushes.end());
            }
            
            static InnerIterator beginInner(BrushList::const_iterator it) {
                Brush* brush = *it;
                return brush->faces().begin();
            }
            
            static InnerIterator endInner(BrushList::const_iterator it) {
                Brush* brush = *it;
                return brush->faces().end();
            }
        };
    }
}

#endif
