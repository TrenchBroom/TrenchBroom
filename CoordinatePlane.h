//
//  CoordinatePlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math3D.h"

@class Vector3f;

@interface CoordinatePlane : NSObject {
    @private
    EPlane3D plane;
}

+ (CoordinatePlane *)projectionPlaneForNormal:(Vector3f *)theNorm;

- (id)initAs:(EPlane3D)thePlane;

- (Vector3f *)project:(Vector3f *)thePoint;
- (BOOL)clockwise:(Vector3f *)theNorm;

- (float)xOf:(Vector3f *)thePoint;
- (float)yOf:(Vector3f *)thePoint;
- (float)zOf:(Vector3f *)thePoint;

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z;

@end
