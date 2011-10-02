/*
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
    NSMutableArray* faces;
    NSMutableArray* partialBrushes;
    NSMutableArray* brushes;
    NSMutableArray* entities;
    NSMutableArray* textureMRU;
    id <Entity> brushSelectionEntity;
    BOOL brushSelectionEntityValid;
    ESelectionMode mode;
}

- (id)initWithUndoManager:(NSUndoManager *)theUndoManager;

- (void)addTexture:(NSString *)texture;
- (void)addFace:(id <Face>)face record:(BOOL)record;
- (void)addFaces:(NSArray *)theFaces record:(BOOL)record;
- (void)addBrush:(id <Brush>)brush record:(BOOL)record;
- (void)addBrushes:(NSArray *)theBrushes record:(BOOL)record;
- (void)addEntity:(id <Entity>)entity record:(BOOL)record;
- (void)addEntities:(NSArray *)theEntities record:(BOOL)record;

- (ESelectionMode)mode;
- (BOOL)isFaceSelected:(id <Face>)face;
- (BOOL)isBrushSelected:(id <Brush>)brush;
- (BOOL)isEntitySelected:(id <Entity>)entity;
- (BOOL)isBrushPartiallySelected:(id <Brush>)brush;

- (NSArray *)textureMRU;
- (NSArray *)selectedFaces;
- (NSArray *)selectedBrushFaces;
- (NSArray *)selectedBrushes;
- (NSArray *)partiallySelectedBrushes;
- (NSArray *)selectedEntities;
- (BOOL)selectionCenter:(TVector3f *)result;
- (BOOL)selectionBounds:(TBoundingBox *)result;
- (id <Entity>)brushSelectionEntity;

- (BOOL)hasSelection;
- (BOOL)hasSelectedEntities;
- (BOOL)hasSelectedBrushes;
- (BOOL)hasSelectedFaces;

- (void)removeFace:(id <Face>)face record:(BOOL)record;
- (void)removeFaces:(NSArray *)theFaces record:(BOOL)record;
- (void)removeBrush:(id <Brush>)brush record:(BOOL)record;
- (void)removeBrushes:(NSArray *)theBrushes record:(BOOL)record;
- (void)removeEntity:(id <Entity>)entity record:(BOOL)record;
- (void)removeEntities:(NSArray *)theEntities record:(BOOL)record;

- (void)removeAll:(BOOL)record;

@end
