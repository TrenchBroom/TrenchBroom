//
//  Matrix3f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 09.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Matrix4f;

@interface Matrix3f : NSObject {
    @private
    float* values; // column major
}

- (id)initWithMatrix3f:(Matrix3f *)matrix;

- (void)setIdentity;
- (void)setMatrix3f:(Matrix3f *)matrix;
- (void)setMinorOf:(Matrix4f *)matrix col:(int)col row:(int)row;

- (BOOL)invert;
- (void)adjunct;
- (float)determinant;
- (void)negate;

- (void)add:(Matrix3f *)matrix;
- (void)sub:(Matrix3f *)matrix;
- (void)mul:(Matrix3f *)matrix;
- (void)scale:(float)factor;

- (float *)columnMajor;
@end
