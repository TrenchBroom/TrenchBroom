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

static NSString* MapView3DDefaults = @"3D View";
static NSString* MapView3DDefaultsBackgroundColor = @"Background Color";

@implementation MapView3D

- (void)rendererChanged:(NSNotification *)notification {
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
    
    MapDocument* document = [controller document];
    Map* map = [document map];
    VBOBuffer* vbo = [controller vbo];
    renderer = [[Renderer alloc] initWithMap:map vbo:vbo];
    
    Camera* camera = [controller camera];
    SelectionManager* selectionManager = [controller selectionManager];
    ToolManager* toolManager = [controller toolManager];
    
    [renderer setCamera:camera];
    [renderer setSelectionManager:selectionManager];
    [renderer setToolManager:toolManager];
    
    [renderer addObserver:self selector:@selector(rendererChanged:) name:RendererChanged];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    InputManager* inputManager = [[[self window] windowController] inputManager];
    [inputManager handleKeyDown:theEvent sender:self];
}

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

- (void) drawRect:(NSRect)dirtyRect {
    NSRect bounds = [self frame];
    glViewport(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureManager* textureManager = [[[self window] windowController] textureManager];
    RenderContext* context = [[RenderContext alloc] initWithTextureManager:textureManager mode:RM_TEXTURED];
    
    [renderer updateView:bounds];
    [renderer render:context];
    
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
    [renderer removeObserver:self];
    [renderer release];
    [super dealloc];
}

@end
