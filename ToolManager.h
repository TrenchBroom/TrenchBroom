//
//  ToolManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const FiguresAdded;
extern NSString* const FiguresRemoved;
extern NSString* const FiguresKey;

@class Ray3D;
@class FaceOffsetTool;
@class FaceRotationTool;
@class MapWindowController;

@interface ToolManager : NSObject {
    @private
    MapWindowController* windowController;
    FaceOffsetTool* faceOffsetTool;
    FaceRotationTool* faceRotationTool;
    NSMutableArray* dragReceivers;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (NSArray *)toolsHitByRay:(Ray3D *)theRay;

- (BOOL)startDrag:(Ray3D *)theRay;
- (void)drag:(Ray3D *)theRay;
- (void)endDrag:(Ray3D *)theRay;
- (BOOL)dragActive;

- (void)keyDown:(NSEvent *)theEvent;

@end
