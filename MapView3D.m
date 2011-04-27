//
//  MapView3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapView3D.h"
#import <OpenGl/gl.h>
#import <OpenGL/glu.h>
#import "Renderer.h"
#import "Camera.h"
#import "TextureManager.h"
#import "InputManager.h"
#import "SelectionManager.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "RenderContext.h"
#import "Options.h"
#import "Grid.h"

static NSString* MapView3DDefaults = @"3D View";
static NSString* MapView3DDefaultsBackgroundColor = @"Background Color";

@implementation MapView3D

- (void)rendererChanged:(NSNotification *)notification {
    [self setNeedsDisplay:YES];
}

- (void)gridChanged:(NSNotification *)notification {
    [self setNeedsDisplay:YES];
}

- (void)awakeFromNib {
    [super awakeFromNib];
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(userDefaultsChanged:) 
                                                 name:NSUserDefaultsDidChangeNotification 
                                               object:[NSUserDefaults standardUserDefaults]];
    
    [self userDefaultsChanged:nil];
}

- (BOOL)becomeFirstResponder {
    if ([super becomeFirstResponder]) {
        NSPoint base = [[self window] convertScreenToBase:[NSEvent mouseLocation]];
        NSPoint point = [self convertPointFromBase:base];
        if (NSPointInRect(point, [self visibleRect])) {
            double timestamp = (double)(AbsoluteToDuration(UpTime())) / 1000.0;
            [self mouseEntered:[NSEvent enterExitEventWithType:NSMouseEntered 
                                                      location:base 
                                                 modifierFlags:[NSEvent modifierFlags] 
                                                     timestamp:timestamp windowNumber:[[self window] windowNumber] context:[[self window] graphicsContext] eventNumber:0 trackingNumber:0 userData:NULL]];
        }
        return YES;
    }
    return NO;
}

- (void)setup {
    MapWindowController* controller = [[self window] windowController];
    
    renderer = [[Renderer alloc] initWithWindowController:controller];
    options = [[controller options] retain];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(rendererChanged:) name:RendererChanged object:renderer];
    [center addObserver:self selector:@selector(gridChanged:) name:GridChanged object:[options grid]];

    mouseTracker = [[NSTrackingArea alloc] initWithRect:[self visibleRect] options:NSTrackingActiveWhenFirstResponder | NSTrackingMouseEnteredAndExited owner:self userInfo:nil];
    [self addTrackingArea:mouseTracker];
}

-  (void)updateTrackingAreas {
    if (mouseTracker != nil) {
        [self removeTrackingArea:mouseTracker];
        [mouseTracker release];
    }
    mouseTracker = [[NSTrackingArea alloc] initWithRect:[self visibleRect] options:NSTrackingActiveWhenFirstResponder | NSTrackingMouseEnteredAndExited owner:self userInfo:nil];
    [self addTrackingArea:mouseTracker];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    if (![inputManager handleKeyDown:theEvent sender:self])
        [super keyDown:theEvent];
}
 
- (void)flagsChanged:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleFlagsChanged:theEvent sender:self];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleLeftMouseDragged:theEvent sender:self];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleMouseMoved:theEvent sender:self];
}

- (void)mouseEntered:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleMouseEntered:theEvent sender:self];
}

- (void)mouseExited:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleMouseExited:theEvent sender:self];
}

- (void)mouseDown:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleLeftMouseDown:theEvent sender:self];
}

- (void)mouseUp:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleLeftMouseUp:theEvent sender:self];
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleRightMouseDragged:theEvent sender:self];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleRightMouseDown:theEvent sender:self];
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleRightMouseUp:theEvent sender:self];
}

- (void)scrollWheel:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleScrollWheel:theEvent sender:self];
}

- (void)beginGestureWithEvent:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleBeginGesture:theEvent sender:self];
}

- (void)endGestureWithEvent:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleEndGesture:theEvent sender:self];
}

- (void)magnifyWithEvent:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleMagnify:theEvent sender:self];
}

- (void) drawRect:(NSRect)dirtyRect {
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera* camera = [[[self window] windowController] camera];
    [camera updateView:[self visibleRect]];
    [renderer render];
    
    [[self openGLContext] flushBuffer];
}

- (void)userDefaultsChanged:(NSNotification *)notification {
    NSDictionary* viewDefaults = [[NSUserDefaults standardUserDefaults] dictionaryForKey:MapView3DDefaults];
    if (viewDefaults == nil)
        return;
    
    NSArray* backgroundColorArray = [viewDefaults objectForKey:MapView3DDefaultsBackgroundColor];
    if (backgroundColorArray != nil && [backgroundColorArray count] == 3)
        for (int i = 0; i < 3; i++)
            backgroundColor[i] = [[backgroundColorArray objectAtIndex:i] floatValue];

    [self setNeedsDisplay:YES];
}

- (Renderer *)renderer {
    return renderer;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [renderer release];
    [options release];
    if (mouseTracker != nil) {
        [self removeTrackingArea:mouseTracker];
        [mouseTracker release];
    }
    [super dealloc];
}

@end
