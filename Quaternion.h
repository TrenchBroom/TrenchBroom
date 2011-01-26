//
//  Quaternion.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Quaternion : NSObject {
    @private
    float a;
    Vector3f* v;
}

+ (Quaternion *)quaternionWithScalar:(float)theScalar vector:(Vector3f *)theVector;
+ (Quaternion *)quaternionWithAngle:(float)theAngle axis:(Vector3f *)theAxis;
+ (Quaternion *)mul:(Quaternion *)left with:(Quaternion *)right;
+ (Quaternion *)conjugate:(Quaternion *)quaternion;

- (id)initWithQuaternion:(Quaternion *)quaternion;
- (id)initWithScalar:(float)theScalar vector:(Vector3f *)theVector;
- (id)initWithAngle:(float)theAngle axis:(Vector3f *)theAxis;

- (float)scalar;
- (Vector3f *)vector;

- (void)setScalar:(float)theScalar;
- (void)setVector:(Vector3f *)theVector;
- (void)setQuaternion:(Quaternion *)quaternion;
- (void)setAngle:(float)theAngle axis:(Vector3f *)theAxis;

- (void)mul:(Quaternion *)right;
- (void)conjugate;

- (void)rotate:(Vector3f *)theVector;

@end
