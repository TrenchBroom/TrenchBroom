/*!
 @header    ConvexArea
 @abstract  A convex area is a possible unbounded area. The edges of the area are stored in a doubly linked list.
 @copyright Kristian Duske 2010
 */


#import <Cocoa/Cocoa.h>
#import "Line.h"
#import "Edge.h"

@interface ConvexArea : NSObject {
    Vector3f* normal;
    Edge* startEdge;
    int numEdges;
}

/*!
    @method     initWithNormal
    @abstract   Initializes a newly allocated convex area by setting its normal vector to a given normal vector.
    @discussion A convex area is a 2D construct, but it needs a normal vector in order to perform operations like determining the side on which a point resides in relation to a given line.
    @param      n The normal vector.
    @result     A convex array that has the given normal vector.
    @throws     NSInvalidArgumentException If the given vector is nil.
*/
- (id)initWithNormal:(Vector3f *)n;

/*!
 @method    addLine
 @abstract  Adds the given line to the boundary of this area.
 @param     line The line to add.
 @result    TRUE if the convex area did not degenerate to a line, point or empty set after adding the given line and FALSE otherwise.
 @throws    NSInvalidArgumentException If the given line is nil.
 */
- (BOOL)addLine:(Line *)line;

/*!
 @method    vertices
 @abstract  Returns an array containing the vertices of this area in counter clockwise order.
 @result    The vertex array.
 */
- (NSArray *)vertices;

/*!
 @method    edges
 @abstract  Returns an array containing the boundary lines of this area in counter clockwise order.
 @result    The line array.
 */
- (NSArray *)edges;

@end
