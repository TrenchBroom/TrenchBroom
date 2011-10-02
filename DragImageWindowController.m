/*
This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "DragImageWindowController.h"
#import "MapView3D.h"

@interface DragImageWindowController (private)

- (BOOL)containsMapViewUnderCursor:(NSView *)theView atLocation:(NSPoint)theLocation;
- (void)finishDrag;

@end

@implementation DragImageWindowController (private)

- (BOOL)containsMapViewUnderCursor:(NSView *)theView atLocation:(NSPoint)theLocation {
    if ([theView isKindOfClass:[MapView3D class]])
        return YES;
    
    NSEnumerator* viewEn = [[theView subviews] objectEnumerator];
    NSView* subview;
    while ((subview = [viewEn nextObject])) {
        NSPoint viewLocation = [subview convertPoint:theLocation fromView:nil];
        if ([subview hitTest:viewLocation] && [self containsMapViewUnderCursor:subview atLocation:theLocation])
            return YES;
    }
    
    return NO;
}

- (void)finishDrag {
    [imageView setImage:nil];
    [[self window] orderOut:self];
}

@end

@implementation DragImageWindowController

- (id)initWithWindow:(NSWindow *)window {
    if (([super initWithWindow:window])) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc {
    [fadeInAnimation release];
    [fadeOutAnimation release];
    [super dealloc];
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    [[self window] setOpaque:NO];
    [[self window] setBackgroundColor:[NSColor clearColor]];
    [[self window] setIgnoresMouseEvents:YES];
    
    NSDictionary* fadeInAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[self window], NSViewAnimationTargetKey, NSViewAnimationFadeInEffect, NSViewAnimationEffectKey, nil];
    fadeInAnimation = [[NSViewAnimation alloc] initWithViewAnimations:[NSArray arrayWithObject:fadeInAttributes]];
    
    NSDictionary* fadeOutAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[self window], NSViewAnimationTargetKey, NSViewAnimationFadeOutEffect, NSViewAnimationEffectKey, nil];
    fadeOutAnimation = [[NSViewAnimation alloc] initWithViewAnimations:[NSArray arrayWithObject:fadeOutAttributes]];
}

- (void)beginDrag:(NSImage *)theImage mouseLocation:(NSPoint)theLocation {
    [[self window] setFrame:NSMakeRect(theLocation.x, theLocation.y, [theImage size].width, [theImage size].height) display:YES];
    slideBackRect = [[self window] frame];
    [[self window] orderFront:self];
    [imageView setImage:theImage];
}

- (void)dragTo:(NSPoint)theLocation {
    NSEnumerator* windowEn = [[NSApp orderedWindows] objectEnumerator];
    NSWindow* window;
    BOOL mapViewActive = NO;
    while ((window = [windowEn nextObject])) {
        if (window != [self window]) {
            NSView* content = [window contentView];
            NSPoint windowLocation = [window mouseLocationOutsideOfEventStream];
            NSPoint viewLocation = [content convertPoint:windowLocation fromView:nil];
            if ([content hitTest:viewLocation]) {
                mapViewActive = [self containsMapViewUnderCursor:content atLocation:windowLocation];
                break;
            }
        }
    }
    
    [[self window] setFrameOrigin:theLocation];
    if (mapViewActive) {
/*
        if ([[self window] alphaValue] > 0) {
            if ([fadeInAnimation isAnimating])
                [fadeInAnimation stopAnimation];
            if (![fadeOutAnimation isAnimating])
                [fadeOutAnimation startAnimation];
        }
 */
        [[self window] orderOut:self];
    } else {
/*
        if ([[self window] alphaValue] < 1) {
            if ([fadeOutAnimation isAnimating])
                [fadeOutAnimation stopAnimation];
            if (![fadeInAnimation isAnimating])
                [fadeInAnimation startAnimation];
        }
 */
        [[self window] orderFront:self];
    }
}

- (void)endDragWithOperation:(NSDragOperation)theOperation {
    if (theOperation == NSDragOperationNone) {
        [[self window] orderFront:self];
        NSDictionary* slideBackAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[self window], NSViewAnimationTargetKey, [NSValue valueWithRect:slideBackRect], NSViewAnimationEndFrameKey, nil];
        NSViewAnimation* slideBackAnimation = [[NSViewAnimation alloc] initWithViewAnimations:[NSArray arrayWithObject:slideBackAttributes]];
        [slideBackAnimation setDelegate:self];
        [slideBackAnimation startAnimation];
        [slideBackAnimation release];
    } else {
        [self finishDrag];
    }
}

- (void)animationDidEnd:(NSAnimation *)animation {
    [self finishDrag];
}

@end
