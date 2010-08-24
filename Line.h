/*!
    @header Line
    @abstract A line in 3-space is defined by a point P and a direction vector D, which is always normalized.
    @copyright Kristian Duske 2010
*/

#import <Cocoa/Cocoa.h>
#import "Math3D.h";

@interface Line : NSObject {
    Vector3f* p;
    Vector3f* d; // normalized
}

- (id)initWithPoint:(Vector3f *)point normalizedDirection:(Vector3f *)dir;
- (id)initWithPoint:(Vector3f *)point direction:(Vector3f *)dir;

- (Vector3f *)p;
- (Vector3f *)d;

- (Vector3f *)intersectionWithLine:(Line *)line;

/*!
    @method     sideOfPoint
    @abstract   Determines on which side of this line a given point resides in relation to a given normal vector.
    @param      point The point to check.
    @param      normal The normal vector, which must be normalized.
    @result     A value of the @link Side enum which indicates the side on which the given point resides.
    @throws     NSInvalidArgumentException if any of the given parameters is nil
*/
- (Side)sideOfPoint:(Vector3f *)point normal:(Vector3f *)normal;

/*!
 @method     turnDirectionTo
 @abstract   Determines in which direction this line needs to be rotated to give it the same direction as a given line in relation to a given normal vector.
 @param      point The line to rotate to.
 @param      normal The normal vector, which must be normalized.
 @result     A value of the @link Side enum which indicates the side on which the given point resides.
 @throws     NSInvalidArgumentException if any of the given parameters is nil
 */
- (Side)turnDirectionTo:(Line *)line normal:(Vector3f *)normal;

/*!
    @method     sameDirectionAs
    @abstract   Determines whether the angle between the direction vectors of this line and the given line is less than PI/2 when seen from above.
    @param      l The other line.
    @param      normal The normal vector, which must be normalized.
    @result     TRUE if the angle of the direction vectors is less than PI/2 and FALSE otherwise
    @throws     NSInvalidArgumentException if the given line is nil
*/
- (BOOL)sameDirectionAs:(Line *)l normal:(Vector3f *)normal;

@end
