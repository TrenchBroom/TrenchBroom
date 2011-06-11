//
//  EntityModelFrame.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityModel.h"
#import "Math.h"

@interface EntityModelFrame : NSObject {
    NSString* name;
    TFrameTriangle* triangles;
    int triangleCount;
    TBoundingBox bounds;
}

- (id)initWithName:(NSString *)theName triangles:(TFrameTriangle *)theTriangles triangleCount:(int)theTriangleCount;

- (NSString *)name;
- (int)triangleCount;
- (const TFrameTriangle *)triangleAtIndex:(int)theIndex;

@end
