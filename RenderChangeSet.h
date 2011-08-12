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
}

- (void)entitiesAdded:(NSSet *)theEntities;
- (void)entitiesRemoved:(NSSet *)theEntities;
- (void)entitiesChanged:(NSSet *)theEntities;
- (void)brushesAdded:(NSSet *)theBrushes;
- (void)brushesRemoved:(NSSet *)theBrushes;
- (void)brushesChanged:(NSSet *)theBrushes;
- (void)clear;

- (NSSet *)addedEntities;
- (NSSet *)removedEntities;
- (NSSet *)changedEntities;
- (NSSet *)addedBrushes;
- (NSSet *)removedBrushes;
- (NSSet *)changedBrushes;

@end
