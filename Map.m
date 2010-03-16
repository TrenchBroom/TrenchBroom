//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Map.h"

NSString* const MapEntityAddedNotification = @"EntityAdded";
NSString* const MapEntityRemovedNotification = @"EntityRemoved";

NSString* const MapEntity = @"Entity";

@implementation Map

- (id)init {
    if (self = [super init]) {
        worldspawn = [[Entity alloc] initWithProperty:@"classname" value:@"worldspawn"];
        entities = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (Entity *)worldspawn {
    return worldspawn;
}

- (void)dealloc {
    [entities release];
    [worldspawn release];
    
    [super dealloc];
}

@end
