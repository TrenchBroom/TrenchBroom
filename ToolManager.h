//
//  ToolManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const FiguresAdded;
extern NSString* const FiguresRemoved;
extern NSString* const FiguresKey;

@class SelectionManager;
@class Ray3D;
@class FaceOffsetTool;
@class Options;

@interface ToolManager : Observable {
    @private
    FaceOffsetTool* faceOffsetTool;
    SelectionManager* selectionManager;
    NSUndoManager* undoManager;
    NSMutableArray* dragReceivers;
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager undoManager:(NSUndoManager *)theUndoManager options:(Options *)theOptions;

- (NSArray *)toolsHitByRay:(Ray3D *)theRay;

- (BOOL)startDrag:(Ray3D *)theRay;
- (void)drag:(Ray3D *)theRay;
- (void)endDrag:(Ray3D *)theRay;
- (BOOL)dragActive;

- (void)keyDown:(NSEvent *)theEvent;

@end
