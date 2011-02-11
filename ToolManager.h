//
//  ToolManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const ToolsAdded;
extern NSString* const ToolsRemoved;
extern NSString* const ToolsKey;

@class SelectionManager;
@class Ray3D;

@interface ToolManager : Observable {
    @private
    NSMutableDictionary* activeTools;
    SelectionManager* selectionManager;
    NSMutableArray* dragReceivers;
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager;

- (NSArray *)toolsHitByRay:(Ray3D *)theRay;

- (BOOL)startDrag:(Ray3D *)theRay;
- (void)drag:(Ray3D *)theRay;
- (void)endDrag:(Ray3D *)theRay;
- (BOOL)dragActive;

@end
