//
//  Math2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    SLeft, SRight, SUp, SDown, SNeither
} ESide2D;

@class Vector2f;

@interface Math2D : NSObject {
    
}

/*!
 @method     turnDirectionFrom
 @abstract   Determines the direction in which a vector must be rotated to match another vector in relation to a given normal vector. All vectors are expected to be normalized.
 @param      from The vector that is to be rotated.
 @param      to The vector to rotate the from vector to.
 @throws     NSInvalidArgumentException if any of the given vectors is nil.
 */
+ (ESide2D)turnDirectionFrom:(Vector2f *)from to:(Vector2f *)to;

@end