//
//  SelectionManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

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

@interface SelectionManager : NSObject {
    @private
    NSMutableSet* faces;
    NSMutableSet* brushes;
    NSMutableSet* entities;
    ESelectionMode mode;
}

- (void)addFace:(Face *)face;
- (void)addFaces:(NSSet *)theFaces;
- (void)addBrush:(Brush *)brush;
- (void)addBrushes:(NSSet *)theBrushes;
- (void)addEntity:(Entity *)entity;
- (void)addEntities:(NSSet *)theEntities;

- (ESelectionMode)mode;
- (BOOL)isFaceSelected:(Face *)face;
- (BOOL)isBrushSelected:(Brush *)brush;
- (BOOL)isEntitySelected:(Entity *)entity;
- (BOOL)hasSelectedFaces:(Brush *)brush;

- (NSSet *)selectedEntities;
- (NSSet *)selectedBrushes;
- (NSSet *)selectedFaces;
- (NSSet *)selectedBrushFaces;

- (BOOL)hasSelection;
- (BOOL)hasSelectedEntities;
- (BOOL)hasSelectedBrushes;
- (BOOL)hasSelectedFaces;

- (void)removeFace:(Face *)face;
- (void)removeBrush:(Brush *)brush;
- (void)removeEntity:(Entity *)entity;

- (void)removeAll;

@end
