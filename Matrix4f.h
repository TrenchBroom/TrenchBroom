//
//  Matrix4f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

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

- (void)rotateAbout:(TVector3f *)axis angle:(float)a;
- (void)translate:(TVector3f *)offset;
- (void)scaleV3f:(TVector3f *)factors;
- (BOOL)invert;
- (void)adjugate;
- (float)determinant;
- (void)transpose;

- (void)setColumn:(int)col row:(int)row value:(float)value;
- (void)setColumn:(int)col values:(const TVector3f *)vector;
- (void)setRow:(int)row values:(const TVector3f *)vector;
- (void)embed:(Matrix3f *)matrix;

- (void)transformVector3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)transformVector4f:(const TVector4f *)vector result:(TVector4f *)result;

- (void)add:(Matrix4f *)matrix;
- (void)sub:(Matrix4f *)matrix;
- (void)mul:(Matrix4f *)matrix;
- (void)scale:(float)factor;

- (float*)columnMajor;
@end
