//
//  Ray3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Ray3D : NSObject {
    @private
    Vector3f* origin;
    Vector3f* direction;
}

- (id)initWithOrigin:(Vector3f *)theOrigin direction:(Vector3f *)theDirection;
- (id)initWithOrigin:(Vector3f *)theOrigin point:(Vector3f *)thePoint;

- (Vector3f *)origin;
- (Vector3f *)direction;

- (Vector3f *)pointAtDistance:(float)distance;

@end
