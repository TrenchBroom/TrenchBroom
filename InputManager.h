//
//  InputManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Camera;

@interface InputManager : NSObject {
}

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (void)handleMouseDragged:(NSEvent *)event sender:(id)sender;

@end
