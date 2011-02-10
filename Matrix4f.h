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
@class Matrix2f;
@class Matrix3f;

@interface Matrix4f : NSObject {
    @private
    float* values; // column major
}

- (id)initWithMatrix4f:(Matrix4f *)matrix;

- (void)setIdentity;
- (void)setMatrix4f:(Matrix4f *)matrix;
- (void)setSubMatrix:(int)index to:(Matrix2f *)matrix;

- (void)rotateAbout:(Vector3f *)axis angle:(float)a;
- (void)translate:(Vector3f *)offset;
- (BOOL)invert;
- (void)adjunct;
- (float)determinant;

- (void)setColumn:(int)col row:(int)row value:(float)value;
- (void)setColumn:(int)col values:(Vector3f *)vector;
- (void)setRow:(int)row values:(Vector3f *)vector;
- (void)embed:(Matrix3f *)matrix;

- (void)transformVector3f:(Vector3f *)vector;
- (void)transformVector4f:(Vector4f *)vector;

- (void)add:(Matrix4f *)matrix;
- (void)sub:(Matrix4f *)matrix;
- (void)mul:(Matrix4f *)matrix;
- (void)scale:(float)factor;

- (float*)columnMajor;
@end
