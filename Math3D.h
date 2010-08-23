//
//  Math3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.05.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"

extern float const AlmostZero;


/*!
 @enum       Side
 @abstract   Enumerates sides.
 */
typedef enum {
    SLeft, SRight, SUp, SDown, SFront, SBack, SNeither
} Side;

@interface Math3D : NSObject {

}

/*!
    @method     turnDirectionFrom
    @abstract   Determines the direction in which a vector must be rotated to match another vector in relation to a given up vector. All vectors are expected to be normalized.
    @param      from The vector that is to be rotated.
    @param      to The vector to rotate the from vector to.
    @param      up The up vector.
    @throws     NSInvalidArgumentException if any of the given vectors is nil or if the given from and up vectors are colinear.
*/
+ (Side)turnDirectionFrom:(Vector3f *)from to:(Vector3f *)to up:(Vector3f *)up;

@end
