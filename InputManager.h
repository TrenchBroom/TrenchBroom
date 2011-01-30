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

@interface InputManager : NSObject {
    @private 
    Picker* picker;
    PickingHit* lastHit;
}

- (id)initWithPicker:(Picker *)thePicker;

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (void)handleMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender;
- (void)handleMouseDown:(NSEvent *)event sender:(id)sender;
- (void)handleMouseUp:(NSEvent *)event sender:(id)sender;
- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender;
@end
