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
#import "RenderMap.h"
#import "Camera.h"
#import "RenderContext.h"
#import "RenderContext3D.h"
#import "TextureManager.h"
#import "InputManager.h"

NSString* const MapView3DDefaults = @"3D View";
NSString* const MapView3DDefaultsBackgroundColor = @"BackgroundColor";

@implementation MapView3D

- (void)awakeFromNib {
    [super awakeFromNib];
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(userDefaultsChanged:) 
                                                 name:NSUserDefaultsDidChangeNotification 
                                               object:[NSUserDefaults standardUserDefaults]];
    
    [self userDefaultsChanged:nil];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    [inputManager handleKeyDown:theEvent sender:self];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    [inputManager handleMouseDragged:theEvent sender:self];
}

- (void) drawRect:(NSRect)dirtyRect {
    NSRect bounds = [self frame];
    glViewport(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderContext3D* renderContext = [[RenderContext3D alloc] initWithTextureManager:textureManager];
    [renderContext preRender];
    [renderContext updateView:bounds withCamera:camera];
    
    [renderMap renderWithContext:renderContext];

    [renderContext postRender];
    [renderContext release];
    
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

- (void)cameraChanged:(NSNotification *)notification {
    [self setNeedsDisplay:YES];
}

- (void)setCamera:(Camera *)aCamera {
    if (aCamera == nil)
        [NSException raise:NSInvalidArgumentException format:@"camera must not be nil"];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    if (camera != nil) {
        [center removeObserver:self name:CameraChanged object:camera];
        [camera release];
    }
    
    camera = [aCamera retain];
    [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
}

- (void)setRenderMap:(RenderMap *)aRenderMap {
    if (aRenderMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"rendermap must not be nil"];
    
    [renderMap release];
    renderMap = [aRenderMap retain];
}

- (void)setTextureManager:(TextureManager *)theTextureManager {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    
    [textureManager release];
    textureManager = [theTextureManager retain];
}

- (void)setInputManager:(InputManager *)theInputManager {
    if (theInputManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"input manager must not be nil"];
    
    [inputManager release];
    inputManager = [theInputManager retain];
}

- (Camera *)camera {
    return camera;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [renderMap release];
    [camera release];
    [textureManager release];
    [inputManager release];
    [super dealloc];
}

@end
