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
        changedFaces = [[NSMutableSet alloc] init];
        selectedEntities = [[NSMutableSet alloc] init];
        deselectedEntities = [[NSMutableSet alloc] init];
        selectedBrushes = [[NSMutableSet alloc] init];
        deselectedBrushes = [[NSMutableSet alloc] init];
        selectedFaces = [[NSMutableSet alloc] init];
        deselectedFaces = [[NSMutableSet alloc] init];
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
    [changedFaces release];
    [selectedEntities release];
    [deselectedEntities release];
    [selectedBrushes release];
    [deselectedBrushes release];
    [selectedFaces release];
    [deselectedFaces release];
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

- (void)facesChanged:(NSSet *)theFaces {
    [changedFaces unionSet:theFaces];
}

- (void)entitiesSelected:(NSSet *)theEntities {
    [selectedEntities unionSet:theEntities];
}

- (void)entitiesDeselected:(NSSet *)theEntities {
    [deselectedEntities unionSet:theEntities];
}

- (void)brushesSelected:(NSSet *)theBrushes {
    [selectedBrushes unionSet:theBrushes];
}

- (void)brushesDeselected:(NSSet *)theBrushes {
    [deselectedBrushes unionSet:theBrushes];
}

- (void)facesSelected:(NSSet *)theFaces {
    [selectedFaces unionSet:theFaces];
}

- (void)facesDeselected:(NSSet *)theFaces {
    [deselectedFaces unionSet:theFaces];
}

- (void)setFilterChanged:(BOOL)isFilterChanged {
    filterChanged = isFilterChanged;
}

- (void)clear {
    [addedEntities removeAllObjects];
    [removedEntities removeAllObjects];
    [changedEntities removeAllObjects];
    [addedBrushes removeAllObjects];
    [removedBrushes removeAllObjects];
    [changedBrushes removeAllObjects];
    [changedFaces removeAllObjects];
    [selectedEntities removeAllObjects];
    [deselectedEntities removeAllObjects];
    [selectedBrushes removeAllObjects];
    [deselectedBrushes removeAllObjects];
    [selectedFaces removeAllObjects];
    [deselectedFaces removeAllObjects];
    filterChanged = NO;
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
    return addedBrushes;
}

- (NSSet *)removedBrushes {
    return removedBrushes;
}

- (NSSet *)changedBrushes {
    return changedBrushes;
}

- (NSSet *)changedFaces {
    return changedFaces;
}

- (NSSet *)selectedEntities {
    return selectedEntities;
}

- (NSSet *)deselectedEntities {
    return deselectedEntities;
}

- (NSSet *)selectedBrushes {
    return selectedBrushes;
}

- (NSSet *)deselectedBrushes {
    return deselectedBrushes;
}

- (NSSet *)selectedFaces {
    return selectedFaces;
}

- (NSSet *)deselectedFaces {
    return deselectedFaces;
}

- (BOOL)filterChanged {
    return filterChanged;
}

@end
