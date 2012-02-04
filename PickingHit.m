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

#import "PickingHit.h"

@implementation PickingHit

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance {
    if ((self = [self init])) {
        object = theObject;
        type = theType;
        hitPoint = *theHitPoint;
        distance = theDistance;
    }
    
    return self;
}

- (id)initWithObject:(id)theObject vertex:(int)theVertexIndex hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance {
    if ((self = [self init])) {
        object = theObject;
        type = HT_VERTEX;
        vertexIndex = theVertexIndex;
        hitPoint = *theHitPoint;
        distance = theDistance;
    }
    
    return self;
}

- (id)object {
    return object;
}

- (EHitType)type {
    return type;
}

- (int)vertexIndex {
    return vertexIndex;
}

- (BOOL)isType:(EHitType)theTypeMask {
    return (theTypeMask & type) != 0;
}

- (const TVector3f *)hitPoint {
    return &hitPoint;
}

- (float)distance {
    return distance;
}

- (NSComparisonResult)compareTo:(PickingHit *)other {
    if (distance < [other distance])
        return NSOrderedAscending;
    if (distance > [other distance])
        return NSOrderedDescending;
    if (type > [other type])
        return NSOrderedAscending;
    if (type < [other type])
        return NSOrderedDescending;
    return NSOrderedSame;
}

@end
