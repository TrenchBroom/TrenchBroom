//
//  CoordinatePlane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "CoordinatePlaneXY.h"
#import "Vector3f.h"

@implementation CoordinatePlaneXY

- (BOOL)clockwise:(Vector3f *)theNorm {
    return [theNorm z] < 0;
}

- (Vector3f *)project:(Vector3f *)thePoint {
    return [[[Vector3f alloc] initWithFloatVector:thePoint] autorelease];
}

- (float)xOf:(Vector3f *)thePoint {
    return [thePoint x];
}

- (float)yOf:(Vector3f *)thePoint {
    return [thePoint y];
}

- (float)zOf:(Vector3f *)thePoint {
    return [thePoint z];
}

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z {
    [thePoint setX:x y:y z:z];
}

@end
