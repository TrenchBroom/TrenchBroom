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
    CM_FRONT,
    CM_BACK,
    CM_SPLIT
} EClipMode;

@class PickingHitList;
@class MutableFace;
@protocol Face;
@protocol Brush;

@interface ClipPlane : NSObject {
    TVector3i points[3];
    PickingHitList* hitLists[3];
    int numPoints;
    EClipMode clipMode;
}

- (void)addPoint:(TVector3i *)thePoint hitList:(PickingHitList *)theHitList;
- (void)updatePoint:(int)index x:(int)x y:(int)y z:(int)z;
- (void)removeLastPoint;
- (int)numPoints;
- (TVector3i *)point:(int)index;
- (PickingHitList *)hitList:(int)index;

- (void)setClipMode:(EClipMode)theClipMode;
- (EClipMode)clipMode;

- (MutableFace *)face:(BOOL)front;
- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult;
- (void)reset;

@end
