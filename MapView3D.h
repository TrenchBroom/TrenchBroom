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
@class VBOBuffer;
@class InputManager;
@class Camera;
@class RenderMap;

@interface MapView3D : NSOpenGLView {
    @private
    float backgroundColor[3];
    RenderMap* renderMap;
    Camera* camera;
    TextureManager* textureManager;
    InputManager* inputManager;
    VBOBuffer* vboBuffer;
}

- (void)setCamera:(Camera *)aCamera;
- (void)setRenderMap:(RenderMap *)aRenderMap;
- (void)setVBOBuffer:(VBOBuffer *)theVboBuffer;
- (void)setTextureManager:(TextureManager *)theTextureManager;
- (void)setInputManager:(InputManager *)theInputManager;

- (Camera *)camera;

- (void)userDefaultsChanged:(NSNotification *)notification;

@end
