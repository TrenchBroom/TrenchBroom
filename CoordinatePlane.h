//
//  CoordinatePlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math3D.h"

@class Vector2f;
@class Vector3f;

@interface CoordinatePlane : NSObject {
    @private
    EPlane3D plane;
}

+ (CoordinatePlane *)projectionPlaneForNormal:(Vector3f *)theNorm;

- (id)initAs:(EPlane3D)thePlane;

- (Vector2f *)project:(Vector3f *)thePoint;

@end
