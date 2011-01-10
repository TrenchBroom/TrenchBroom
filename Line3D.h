//
//  Line3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 09.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Line3D : NSObject {
    @private
    Vector3f* point;
    Vector3f* direction; // normalized
}
+ (Line3D *)lineWithPoint1:(Vector3f *)point1 point2:(Vector3f *)point2;
+ (Line3D *)lineWithPoint:(Vector3f *)aPoint normalizedDirection:(Vector3f *)aDirection;
+ (Line3D *)lineWithPoint:(Vector3f *)aPoint direction:(Vector3f *)aDirection;
+ (Line3D *)lineWithLine:(Line3D *)aLine;

- (id)initWithPoint1:(Vector3f *)point1 point2:(Vector3f *)point2;
- (id)initWithPoint:(Vector3f *)aPoint normalizedDirection:(Vector3f *)aDirection;
- (id)initWithPoint:(Vector3f *)aPoint direction:(Vector3f *)aDirection;
- (id)initWithLine:(Line3D *)aLine;

- (Vector3f *)point;
- (Vector3f *)direction;

@end
