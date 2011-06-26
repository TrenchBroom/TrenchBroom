//
//  FaceInfo.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FaceInfo.h"
#import "Face.h"
#import "MutableFace.h"

@implementation FaceInfo

+ (id)faceInfoFor:(id <Face>)theFace {
    return [[[FaceInfo alloc] initWithFace:theFace] autorelease];
}

- (id)initWithFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    
    if ((self = [self init])) {
        faceId = [[theFace faceId] retain];
        point1 = *[theFace point1];
        point2 = *[theFace point2];
        point3 = *[theFace point3];
        xOffset = [theFace xOffset];
        yOffset = [theFace yOffset];
        xScale = [theFace xScale];
        yScale = [theFace yScale];
        texture = [[NSString alloc] initWithString:[theFace texture]];
    }
    
    return self;
}

- (void)dealloc {
    [faceId release];
    [texture release];
    [super dealloc];
}
                   
- (void)updateFace:(MutableFace *)theFace {
    NSAssert([faceId isEqualToNumber:[theFace faceId]], @"face id must be equal");
    
    [theFace setPoint1:&point1 point2:&point2 point3:&point3];
    [theFace setXOffset:xOffset];
    [theFace setYOffset:yOffset];
    [theFace setXScale:xScale];
    [theFace setYScale:yScale];
    [theFace setRotation:rotation];
    [theFace setTexture:texture];
}

@end
