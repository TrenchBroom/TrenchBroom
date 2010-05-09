//
//  Plane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3i.h"
#import "Vector3f.h"
#import "Line.h"

typedef enum {
    PSFront, PSBack
} PlaneSide;

@interface Plane : NSObject {
    float d;
    Vector3f* n; // normalized
}

- (id)initWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3;

- (float)d;
- (Vector3f *)n;

- (Line *)intersectionWithPlane:(Plane *)plane;

@end
