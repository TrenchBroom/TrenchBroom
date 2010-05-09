/*!
    @header Line
    @abstract A line in 3-space is defined by a point P and a direction vector D, which is always normalized.
    @copyright Kristian Duske 2010
*/

#import <Cocoa/Cocoa.h>
#import "Vector3f.h";

/*!
    @enum LineSide
    @abstract   Enumerates the sides of the line on which a point can reside.
    @discussion The sides are determined by viewer that looks in the direction of the vector D. The up vector must be provided.
    @constant   LSLeft The left side of the line.
    @constant   LSRight The right side of the line.
    @constant   LSNeither The point is included in the line.
*/

typedef enum {
    LSLeft, LSRight, LSNeither
} LineSide;

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
    @abstract   Determines on which side of this line a given point resides in relation to a given up vector.
    @param      point The point to check.
    @param      up The up vector, which must be normalized.
    @result     A value of the @link LineSide enum which indicates the side on which the given point resides.
    @throws     NSInvalidArgumentException if any of the given parameters is nil
*/
- (LineSide)sideOfPoint:(Vector3f *)point up:(Vector3f *)up;

/*!
    @method     sameDirectionAs
    @abstract   Determines whether the angle between the direction vectors of this line and the given line is less than PI/2 when seen from above.
    @param      l The other line.
    @param      up The up vector, which must be normalized.
    @result     TRUE if the angle of the direction vectors is less than PI/2 and FALSE otherwise
    @throws     NSInvalidArgumentException if the given line is nil
*/
- (BOOL)sameDirectionAs:(Line *)l up:(Vector3f *)up;

@end
