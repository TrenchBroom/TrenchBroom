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

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    CP_WSB = 0,
    CP_WST = 1,
    CP_WNB = 2,
    CP_WNT = 3,
    CP_ESB = 4,
    CP_EST = 5,
    CP_ENB = 6,
    CP_ENT = 7
    
} EChildPosition;

@interface OctreeNode : NSObject {
    @private
    int minSize;
    TVector3i min;
    TVector3i max;
    NSMutableSet* objects;
    OctreeNode* children[8];
}

- (id)initWithMin:(TVector3i *)theMin max:(TVector3i *)theMax minSize:(int)theMinSize;

- (BOOL)addObject:(id)theObject bounds:(const TBoundingBox *)theBounds;
- (BOOL)removeObject:(id)theObject bounds:(const TBoundingBox *)theBounds;

- (void)addObjectsForRay:(const TRay *)ray to:(NSMutableArray *)list;

@end
