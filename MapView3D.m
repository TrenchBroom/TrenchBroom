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

static NSString* MapView3DDefaults = @"3D View";
static NSString* MapView3DDefaultsBackgroundColor = @"Background Color";

@implementation MapView3D

- (void)rendererChanged:(NSNotification *)notification {
    [self setNeedsDisplay:YES];
}

- (void)optionsChanged:(NSNotification *)notification {
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

- (void)setup {
    MapWindowController* controller = [[self window] windowController];
    
    renderer = [[Renderer alloc] initWithWindowController:controller];
    options = [[controller options] retain];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(rendererChanged:) name:RendererChanged object:renderer];
    [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

/*
- (void)keyDown:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleKeyDown:theEvent sender:self];
}
*/
 
- (void)mouseDragged:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleLeftMouseDragged:theEvent sender:self];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleMouseMoved:theEvent sender:self];
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
    NSRect bounds = [self frame];
    glViewport(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    [renderer updateView:bounds];
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

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [renderer release];
    [options release];
    [super dealloc];
}

@end
