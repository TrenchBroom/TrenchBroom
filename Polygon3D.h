//
//  Polygon3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math3D.h"

@interface Polygon3D : NSObject {
    NSMutableArray* vertices;
}
- (id)init;
- (id)initWithVertices:(NSArray *)vertices;
- (id)initParaxialRectangleAt:(Vector3f *)center dimensions:(Vector2f *)dimensions plane:(Plane3D)plane direction:(Axis3D)axis;
@end
