//
//  RenderChangeSet.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface RenderChangeSet : NSObject {
    NSMutableSet* addedEntities;
    NSMutableSet* removedEntities;
    NSMutableSet* changedEntities;
    NSMutableSet* addedBrushes;
    NSMutableSet* removedBrushes;
    NSMutableSet* changedBrushes;
    NSMutableSet* changedFaces;
    NSMutableSet* selectedEntities;
    NSMutableSet* deselectedEntities;
    NSMutableSet* selectedBrushes;
    NSMutableSet* deselectedBrushes;
    NSMutableSet* selectedFaces;
    NSMutableSet* deselectedFaces;
}

- (void)entitiesAdded:(NSSet *)theEntities;
- (void)entitiesRemoved:(NSSet *)theEntities;
- (void)entitiesChanged:(NSSet *)theEntities;
- (void)brushesAdded:(NSSet *)theBrushes;
- (void)brushesRemoved:(NSSet *)theBrushes;
- (void)brushesChanged:(NSSet *)theBrushes;
- (void)facesChanged:(NSSet *)theFaces;
- (void)entitiesSelected:(NSSet *)theEntities;
- (void)entitiesDeselected:(NSSet *)theEntities;
- (void)brushesSelected:(NSSet *)theBrushes;
- (void)brushesDeselected:(NSSet *)theBrushes;
- (void)facesSelected:(NSSet *)theFaces;
- (void)facesDeselected:(NSSet *)theFaces;
- (void)clear;

- (NSSet *)addedEntities;
- (NSSet *)removedEntities;
- (NSSet *)changedEntities;
- (NSSet *)addedBrushes;
- (NSSet *)removedBrushes;
- (NSSet *)changedBrushes;
- (NSSet *)changedFaces;
- (NSSet *)selectedEntities;
- (NSSet *)deselectedEntities;
- (NSSet *)selectedBrushes;
- (NSSet *)deselectedBrushes;
- (NSSet *)selectedFaces;
- (NSSet *)deselectedFaces;

@end
