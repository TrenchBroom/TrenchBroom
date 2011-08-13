//
//  RenderChangeSet.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface RenderChangeSet : NSObject {
    NSMutableArray* addedEntities;
    NSMutableArray* removedEntities;
    NSMutableArray* changedEntities;
    NSMutableArray* addedBrushes;
    NSMutableArray* removedBrushes;
    NSMutableArray* changedBrushes;
    NSMutableArray* changedFaces;
    NSMutableArray* selectedEntities;
    NSMutableArray* deselectedEntities;
    NSMutableArray* selectedBrushes;
    NSMutableArray* deselectedBrushes;
    NSMutableArray* selectedFaces;
    NSMutableArray* deselectedFaces;
    BOOL filterChanged;
}

- (void)entitiesAdded:(NSArray *)theEntities;
- (void)entitiesRemoved:(NSArray *)theEntities;
- (void)entitiesChanged:(NSArray *)theEntities;
- (void)brushesAdded:(NSArray *)theBrushes;
- (void)brushesRemoved:(NSArray *)theBrushes;
- (void)brushesChanged:(NSArray *)theBrushes;
- (void)facesChanged:(NSArray *)theFaces;
- (void)entitiesSelected:(NSArray *)theEntities;
- (void)entitiesDeselected:(NSArray *)theEntities;
- (void)brushesSelected:(NSArray *)theBrushes;
- (void)brushesDeselected:(NSArray *)theBrushes;
- (void)facesSelected:(NSArray *)theFaces;
- (void)facesDeselected:(NSArray *)theFaces;
- (void)setFilterChanged:(BOOL)isFilterChanged;
- (void)clear;

- (NSArray *)addedEntities;
- (NSArray *)removedEntities;
- (NSArray *)changedEntities;
- (NSArray *)addedBrushes;
- (NSArray *)removedBrushes;
- (NSArray *)changedBrushes;
- (NSArray *)changedFaces;
- (NSArray *)selectedEntities;
- (NSArray *)deselectedEntities;
- (NSArray *)selectedBrushes;
- (NSArray *)deselectedBrushes;
- (NSArray *)selectedFaces;
- (NSArray *)deselectedFaces;
- (BOOL)filterChanged;

@end
