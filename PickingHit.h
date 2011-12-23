/*
Copyright (C) 2010-2011 Kristian Duske

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
    HT_ENTITY       = 1 << 0,
    HT_FACE         = 1 << 1,
    HT_CLOSE_FACE   = 1 << 2,
    HT_VERTEX       = 1 << 3,
    HT_ANY          = HT_ENTITY | HT_FACE
} EHitType;

@interface PickingHit : NSObject {
    id object;
    EHitType type;
    TVector3f hitPoint;
    int vertexIndex;
    float distance;
}

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance;
- (id)initWithObject:(id)theObject vertex:(int)theVertexIndex hitPoint:(const TVector3f *)theHitPoint distance:(float)theDistance;

- (id)object;
- (EHitType)type;
- (int)vertexIndex;
- (BOOL)isType:(EHitType)theTypeMask;
- (const TVector3f *)hitPoint;
- (float)distance;

- (NSComparisonResult)compareTo:(PickingHit *)other;

@end
