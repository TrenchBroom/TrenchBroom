//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Map.h"


@implementation Map

- (id)init {
    if (self = [super init]) {
        worldspawn = [[Entity alloc] initWithKey:@"classname" value:@"worldspawn"];
        entities = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [entities release];
    [worldspawn release];
    
    [super dealloc];
}

@end
