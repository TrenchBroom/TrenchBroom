//
//  Matrix2f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 09.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Matrix4f;
@class Matrix3f;

@interface Matrix2f : NSObject {
    @private
    float* values; // column major
}

- (id)initWithMatrix2f:(Matrix2f *)matrix;
/*!
    @function
    @abstract   Initializes this matrix as a sub matrix of the given 4x4 matrix.
    @discussion The given index indicates which sub matrix is to be initialized. 0 indicates the top left sub matrix, 1 the bottom left, 2 the top right and 3 the bottom right sub matrix.
    @param      index The sub matrix index.
    @param      matrix The 4x4 matrix.
    @result     The initialized sub matrix.
*/
- (id)initAsSubMatrix:(int)index of:(Matrix4f *)matrix;

- (void)setIdentity;
- (void)setMatrix2f:(Matrix2f *)matrix;
- (void)setMinorOf:(Matrix3f *)matrix col:(int)col row:(int)row;

- (BOOL)invert;
- (void)adjunct;
- (float)determinant;
- (void)negate;

- (void)add:(Matrix2f *)matrix;
- (void)sub:(Matrix2f *)matrix;
- (void)mul:(Matrix2f *)matrix;
- (void)scale:(float)factor;

- (float *)columnMajor;
@end
