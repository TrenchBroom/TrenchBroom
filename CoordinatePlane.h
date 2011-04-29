//
//  CoordinatePlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@protocol CoordinatePlane <NSObject>

- (Vector3f *)project:(Vector3f *)thePoint;
- (BOOL)clockwise:(Vector3f *)theNorm;

- (float)xOf:(Vector3f *)thePoint;
- (float)yOf:(Vector3f *)thePoint;
- (float)zOf:(Vector3f *)thePoint;

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z;

@end
