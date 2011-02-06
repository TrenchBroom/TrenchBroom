//
//  Matrix4f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class Vector4f;

@interface Matrix4f : NSObject {
    @private
    float* values; // column major
}

- (void)setIdentity;
- (void)setMatrix4f:(Matrix4f *)matrix;
- (void)rotateAbout:(Vector3f *)axis angle:(float)a;
- (void)translate:(Vector3f *)offset;

- (void)setRow:(int)row column:(int)column value:(float)value;
- (void)setRow:(int)row values:(Vector3f *)vector;

- (void)transformVector3f:(Vector3f *)vector;
- (void)transformVector4f:(Vector4f *)vector;

- (float*)columnMajor;
@end
