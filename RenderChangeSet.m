//
//  RenderChangeSet.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RenderChangeSet.h"

@implementation RenderChangeSet

- (id)init {
    if ((self = [super init])) {
        addedEntities = [[NSMutableSet alloc] init];
        removedEntities = [[NSMutableSet alloc] init];
        changedEntities = [[NSMutableSet alloc] init];
        addedBrushes = [[NSMutableSet alloc] init];
        removedBrushes = [[NSMutableSet alloc] init];
        changedBrushes = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [addedEntities release];
    [removedEntities release];
    [changedEntities release];
    [addedBrushes release];
    [removedBrushes release];
    [changedBrushes release];
    [super dealloc];
}

- (void)entitiesAdded:(NSSet *)theEntities {
    [addedEntities unionSet:theEntities];
}

- (void)entitiesRemoved:(NSSet *)theEntities {
    [removedEntities unionSet:theEntities];
}

- (void)entitiesChanged:(NSSet *)theEntities {
    [changedEntities unionSet:theEntities];
}

- (void)brushesAdded:(NSSet *)theBrushes {
    [addedBrushes unionSet:theBrushes];
}

- (void)brushesRemoved:(NSSet *)theBrushes {
    [removedBrushes unionSet:theBrushes];
}

- (void)brushesChanged:(NSSet *)theBrushes {
    [changedBrushes unionSet:theBrushes];
}

- (void)clear {
    [addedEntities removeAllObjects];
    [removedEntities removeAllObjects];
    [changedEntities removeAllObjects];
    [addedBrushes removeAllObjects];
    [removedBrushes removeAllObjects];
    [changedBrushes removeAllObjects];
}

- (NSSet *)addedEntities {
    return addedEntities;
}

- (NSSet *)removedEntities {
    return removedEntities;
}

- (NSSet *)changedEntities {
    return changedEntities;
}

- (NSSet *)addedBrushes {
    return addedEntities;
}

- (NSSet *)removedBrushes {
    return removedBrushes;
}

- (NSSet *)changedBrushes {
    return changedBrushes;
}

@end
