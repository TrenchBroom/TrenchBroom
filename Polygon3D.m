//
//  Polygon3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon3D.h"


@implementation Polygon3D
- (id)init {
    if (self == [super init])
        vertices = [[NSMutableArray alloc] init];

    return self;
}

- (id)initWithVertices:(NSArray *)vertices {
    if (vertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
        
    if (self = [super init])
        vertices = [[NSMutableArray alloc] initWithArray:vertices];
    
    return self;
}

- (void)dealloc {
    [vertices release];
    [super init];
}
@end
