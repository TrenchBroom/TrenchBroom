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

- (id)initWithObject:(id)theObject vertexIndex:(int)theVertexIndex hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance {
    if ((self = [self init])) {
        object = theObject;
        type = HT_VERTEX_HANDLE;
        index = theVertexIndex;
        hitPoint = *theHitPoint;
        distance = theDistance;
    }
    
    return self;
}

- (id)initWithObject:(id)theObject edgeIndex:(int)theEdgeIndex hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance {
    if ((self = [self init])) {
        object = theObject;
        type = HT_EDGE_HANDLE;
        index = theEdgeIndex;
        hitPoint = *theHitPoint;
        distance = theDistance;
    }
    
    return self;
}

- (id)initWithObject:(id)theObject faceIndex:(int)theFaceIndex hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance {
    if ((self = [self init])) {
        object = theObject;
        type = HT_FACE_HANDLE;
        index = theFaceIndex;
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

- (int)index {
    return index;
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

- (NSString *)description {
    NSString* typeStr;
    switch (type) {
        case HT_ENTITY:
            typeStr = @"HT_ENTITY";
            break;
        case HT_FACE:
            typeStr = @"HT_FACE";
            break;
        case HT_CLOSE_FACE:
            typeStr = @"HT_CLOSE_FACE";
            break;
        case HT_VERTEX_HANDLE:
            typeStr = @"HT_VERTEX_HANDLE";
            break;
        case HT_EDGE_HANDLE:
            typeStr = @"HT_EDGE_HANDLE";
            break;
        case HT_FACE_HANDLE:
            typeStr = @"HT_FACE_HANDLE";
            break;
        default:
            typeStr = @"unknown";
            break;
    }
    
    return [NSString stringWithFormat:@"Type: %@, object %@, hit point %f %f %f, index %i, distance %f", typeStr, object, hitPoint.x, hitPoint.y, hitPoint.z, index, distance];
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
