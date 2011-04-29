//
//  CoordinatePlaneXZ.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CoordinatePlaneXZ.h"
#import "Vector3f.h"

@implementation CoordinatePlaneXZ

- (BOOL)clockwise:(Vector3f *)theNorm {
    return [theNorm y] < 0;
}

- (Vector3f *)project:(Vector3f *)thePoint {
    return [[[Vector3f alloc] initWithFloatX:[thePoint x] y:[thePoint z] z:[thePoint y]] autorelease];
}

- (float)xOf:(Vector3f *)thePoint {
    return [thePoint x];
}

- (float)yOf:(Vector3f *)thePoint {
    return [thePoint z];
}

- (float)zOf:(Vector3f *)thePoint {
    return [thePoint y];
}

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z {
    [thePoint setX:x y:z z:y];
}

@end
