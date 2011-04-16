//
//  BoundingBox.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class Grid;

@interface BoundingBox : NSObject {
    @private
    Vector3f* min;
    Vector3f* max;
    Vector3f* size;
}

- (id)initWithMin:(Vector3f *)theMin max:(Vector3f *)theMax;
- (id)initWithBounds:(BoundingBox *)theBounds;
- (id)initWithBrushes:(NSSet *)theBrushes;

- (Vector3f *)min;
- (Vector3f *)max;

- (Vector3f *)size;
- (Vector3f *)center;

- (void)mergeBounds:(BoundingBox *)theBounds;
- (void)mergeMin:(Vector3f *)theMin max:(Vector3f *)theMax;
- (void)mergePoint:(Vector3f *)thePoint;
- (void)expandBy:(float)delta;
- (void)expandToGrid:(Grid *)theGrid;

@end
