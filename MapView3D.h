//
//  MapView3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const MapView3DDefaults;
extern NSString* const MapView3DDefaultsBackgroundColor;

@class TextureManager;
@class InputManager;
@class Camera;
@class RenderMap;

@interface MapView3D : NSOpenGLView {
    RenderMap* renderMap;
    Camera* camera;
    TextureManager* textureManager;
    float backgroundColor[3];
    InputManager* inputManager;
}

- (void)setCamera:(Camera *)aCamera;
- (void)setRenderMap:(RenderMap *)aRenderMap;
- (void)setTextureManager:(TextureManager *)theTextureManager;
- (void)setInputManager:(InputManager *)theInputManager;

- (Camera *)camera;

- (void)userDefaultsChanged:(NSNotification *)notification;

@end
