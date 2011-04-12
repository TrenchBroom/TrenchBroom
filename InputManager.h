//
//  InputManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Picker;
@class PickingHitList;
@class MapWindowController;
@class ClipTool;
@protocol Tool;

@interface InputManager : NSObject {
    @private 
    PickingHitList* lastHits;
    MapWindowController* windowController;
    id <Tool> tool;
    ClipTool* clipTool;
    BOOL gesture;
    BOOL drag;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (BOOL)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (void)handleFlagsChanged:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender;
- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender;
- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender;
- (void)handleEndGesture:(NSEvent *)event sender:(id)sender;
- (void)handleMagnify:(NSEvent *)event sender:(id)sender;

- (void)setClipTool:(ClipTool *)theClipTool;
@end
