//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"


@implementation Brush

- (id)init {
    
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    
    [faces release];
    [super dealloc];
}

@end
