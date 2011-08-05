//
//  SelectionManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

extern NSString* const SelectionAdded;
extern NSString* const SelectionRemoved;

extern NSString* const SelectionEntities;
extern NSString* const SelectionBrushes;
extern NSString* const SelectionFaces;
extern NSString* const SelectionVertices;

typedef enum {
    SM_UNDEFINED,
    SM_FACES,
    SM_BRUSHES,
    SM_ENTITIES,
    SM_BRUSHES_ENTITIES
} ESelectionMode;

@protocol Face;
@protocol Brush;
@protocol Entity;

@interface SelectionManager : NSObject {
    @private
    NSUndoManager* undoManager;
    NSMutableSet* faces;
    NSMutableSet* partialBrushes;
    NSMutableSet* brushes;
    NSMutableSet* entities;
    id <Entity> brushSelectionEntity;
    BOOL brushSelectionEntityValid;
    ESelectionMode mode;
}

- (id)initWithUndoManager:(NSUndoManager *)theUndoManager;

- (void)addFace:(id <Face>)face record:(BOOL)record;
- (void)addFaces:(NSSet *)theFaces record:(BOOL)record;
- (void)addBrush:(id <Brush>)brush record:(BOOL)record;
- (void)addBrushes:(NSSet *)theBrushes record:(BOOL)record;
- (void)addEntity:(id <Entity>)entity record:(BOOL)record;
- (void)addEntities:(NSSet *)theEntities record:(BOOL)record;

- (ESelectionMode)mode;
- (BOOL)isFaceSelected:(id <Face>)face;
- (BOOL)isBrushSelected:(id <Brush>)brush;
- (BOOL)isEntitySelected:(id <Entity>)entity;
- (BOOL)isBrushPartiallySelected:(id <Brush>)brush;

- (NSSet *)selectedEntities;
- (NSSet *)selectedBrushes;
- (NSSet *)selectedFaces;
- (NSSet *)selectedBrushFaces;
- (NSSet *)partiallySelectedBrushes;
- (BOOL)selectionCenter:(TVector3f *)result;
- (BOOL)selectionBounds:(TBoundingBox *)result;
- (id <Entity>)brushSelectionEntity;

- (BOOL)hasSelection;
- (BOOL)hasSelectedEntities;
- (BOOL)hasSelectedBrushes;
- (BOOL)hasSelectedFaces;

- (void)removeFace:(id <Face>)face record:(BOOL)record;
- (void)removeFaces:(NSSet *)theFaces record:(BOOL)record;
- (void)removeBrush:(id <Brush>)brush record:(BOOL)record;
- (void)removeBrushes:(NSSet *)theBrushes record:(BOOL)record;
- (void)removeEntity:(id <Entity>)entity record:(BOOL)record;
- (void)removeEntities:(NSSet *)theEntities record:(BOOL)record;

- (void)removeAll:(BOOL)record;

@end
