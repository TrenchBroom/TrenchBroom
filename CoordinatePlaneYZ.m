//
//  CoordinatePlaneYZ.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CoordinatePlaneYZ.h"
#import "Vector3f.h"

@implementation CoordinatePlaneYZ

- (BOOL)clockwise:(Vector3f *)theNorm {
    return [theNorm x] > 0;
}

- (Vector3f *)project:(Vector3f *)thePoint {
    return [[[Vector3f alloc] initWithFloatX:[thePoint y] y:[thePoint z] z:[thePoint x]] autorelease];
}

- (float)xOf:(Vector3f *)thePoint {
            return [thePoint y];
}

- (float)yOf:(Vector3f *)thePoint {
            return [thePoint z];
}

- (float)zOf:(Vector3f *)thePoint {
            return [thePoint x];
}

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z {
    [thePoint setX:[thePoint z] y:[thePoint x] z:[thePoint y]];
}

@end
