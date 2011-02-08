//
//  SelectionManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const SelectionAdded;
extern NSString* const SelectionRemoved;

extern NSString* const SelectionEntities;
extern NSString* const SelectionBrushes;
extern NSString* const SelectionFaces;

typedef enum {
    SM_UNDEFINED,
    SM_FACES,
    SM_GEOMETRY
} ESelectionMode;

@class Face;
@class Brush;
@class Entity;

@interface SelectionManager : Observable {
    @private
    NSMutableSet* faces;
    NSMutableSet* brushes;
    NSMutableSet* entities;
    ESelectionMode mode;
}

- (void)addFace:(Face *)face;
- (void)addBrush:(Brush *)brush;
- (void)addEntity:(Entity *)entity;

- (ESelectionMode)mode;
- (BOOL)isFaceSelected:(Face *)face;
- (BOOL)isBrushSelected:(Brush *)brush;
- (BOOL)isEntitySelected:(Entity *)entity;
- (BOOL)hasSelectedFaces:(Brush *)brush;

- (NSSet *)selectedEntities;
- (NSSet *)selectedBrushes;
- (NSSet *)selectedFaces;

- (BOOL)hasSelection;

- (void)removeFace:(Face *)face;
- (void)removeBrush:(Brush *)brush;
- (void)removeEntity:(Entity *)entity;

- (void)removeAll;

@end
