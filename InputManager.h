//
//  InputManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Picker;
@class PickingHit;
@class SelectionManager;
@class ToolManager;

@interface InputManager : NSObject {
    @private 
    Picker* picker;
    PickingHit* lastHit;
    SelectionManager* selectionManager;
    ToolManager* toolManager;
    NSMutableArray* currentTools;
}

- (id)initWithPicker:(Picker *)thePicker selectionManager:(SelectionManager *)theSelectionManager toolManager:(ToolManager *)theToolManager;

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender;
- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender;
@end
