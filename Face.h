//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Brush;
@class Vertex;
@class Vector3i;
@class Vector3f;
@class Vector2f;
@class HalfSpace3D;
@class Ray3D;
@class PickingHit;

@protocol Face <NSObject>

- (NSNumber *)faceId;
- (id <Brush>)brush;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;

- (NSString *)texture;
- (int)xOffset;
- (int)yOffset;
- (float)rotation;
- (float)xScale;
- (float)yScale;

- (Vector3f *)norm;
- (Vector3f *)center;
- (NSArray *)vertices;
- (NSArray *)edges;

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex;
- (Vector3f *)worldCoordsOf:(Vector3f *)sCoords;
- (Vector3f *)surfaceCoordsOf:(Vector3f *)wCoords;
- (HalfSpace3D *)halfSpace;
- (NSArray *)gridWithSize:(int)gridSize;

@end
