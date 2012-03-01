//
//  PreferencesViewAnimation.m
//  TrenchBroom
//
//  Created by Kristian Duske on 01.03.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

#import "PreferencesViewAnimation.h"

@implementation PreferencesViewAnimation

- (id)initWithDuration:(NSTimeInterval)duration animationCurve:(NSAnimationCurve)animationCurve {
    if ((self = [super initWithDuration:duration animationCurve:animationCurve])) {
        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
        started = NO;
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];
    
    NSView* contentView = [window contentView];
    if (!started) {
        [contentView addSubview:inView];
        started = YES;
    }
    
    [outView setAlphaValue:1 - progress];
    [inView setAlphaValue:progress];
    
    NSRect windowFrame = sourceFrame;
    windowFrame.origin.x += (targetFrame.origin.x - sourceFrame.origin.x) * progress;
    windowFrame.origin.y += (targetFrame.origin.y - sourceFrame.origin.y) * progress;
    windowFrame.size.width += (targetFrame.size.width - sourceFrame.size.width) * progress;
    windowFrame.size.height += (targetFrame.size.height - sourceFrame.size.height) * progress;
    
    [window setFrame:windowFrame display:YES];

    NSRect viewFrame = [inView frame];
    viewFrame.origin.y = NSHeight([contentView frame]) - NSHeight(viewFrame);
    [inView setFrame:viewFrame];
}

- (void)animationDidStop:(NSAnimation *)animation {
    [outView removeFromSuperview];
}

- (void)animationDidEnd:(NSAnimation *)animation {
    [outView removeFromSuperview];
}

- (void)setOutView:(NSView *)theOutView {
    outView = theOutView;
}

- (void)setInView:(NSView *)theInView {
    inView = theInView;
}

- (void)setWindow:(NSWindow *)theWindow {
    sourceFrame = [theWindow frame];
    window = theWindow;
}

- (void)setTargetFrame:(NSRect)theTargetFrame {
    targetFrame = theTargetFrame;
}

@end
