//
//  DragImageWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface DragImageWindowController : NSWindowController <NSAnimationDelegate> {
    IBOutlet NSImageView* imageView;
    NSViewAnimation* fadeInAnimation;
    NSViewAnimation* fadeOutAnimation;
    NSRect slideBackRect;
}

- (void)beginDrag:(NSImage *)theImage mouseLocation:(NSPoint)theLocation;
- (void)dragTo:(NSPoint)theLocation;
- (void)endDragWithOperation:(NSDragOperation)theOperation;

@end
