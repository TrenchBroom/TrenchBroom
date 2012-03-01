//
//  PreferencesViewAnimation.h
//  TrenchBroom
//
//  Created by Kristian Duske on 01.03.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface PreferencesViewAnimation : NSAnimation <NSAnimationDelegate> {
    NSView* outView;
    NSView* inView;
    NSWindow* window;
    NSRect sourceFrame;
    NSRect targetFrame;
    BOOL started;
}

- (void)setOutView:(NSView *)theOutView;
- (void)setInView:(NSView *)theInView;
- (void)setWindow:(NSWindow *)theWindow;
- (void)setTargetFrame:(NSRect)theTargetFrame;

@end
