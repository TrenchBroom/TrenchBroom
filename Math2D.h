//
//  Math2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "Vector2f.h"

typedef enum {
    SLeft, SRight, SUp, SDown, SNeither
} Side2D;

@interface Math2D : NSObject {
    
}

/*!
 @method     turnDirectionFrom
 @abstract   Determines the direction in which a vector must be rotated to match another vector in relation to a given normal vector. All vectors are expected to be normalized.
 @param      from The vector that is to be rotated.
 @param      to The vector to rotate the from vector to.
 @throws     NSInvalidArgumentException if any of the given vectors is nil.
 */
+ (Side2D)turnDirectionFrom:(Vector2f *)from to:(Vector2f *)to;

@end