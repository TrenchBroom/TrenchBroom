/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    BOOL textureManagerChanged;
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
- (void)setTextureManagerChanged:(BOOL)isTextureManagerChanged;
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
- (BOOL)textureManagerChanged;

@end
