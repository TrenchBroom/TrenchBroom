//
//  Bsp.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspModel.h"


@implementation BspModel

- (id)initWithFaces:(NSArray *)theFaces textures:(NSArray *)theTextures {
    NSAssert(theFaces != nil, @"face list must not be nil");
    NSAssert([theFaces count] >= 4, @"face list must contain at least 4 faces");
    NSAssert(theTextures != nil, @"texture list must not be nil");
    NSAssert([theTextures count] > 0, @"texture list must not be empty");
    
    if ((self = [self init])) {
        faces = [theFaces retain];
        textures = [theTextures retain];
    }
    
    return self;
}

- (void)dealloc {
    [faces release];
    [textures release];
    [super dealloc];
}

@end
