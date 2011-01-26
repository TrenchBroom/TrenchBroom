//
//  MapWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapView2D;
@class MapView3D;
@class TextureView;
@class Camera;
@class RenderMap;
@class TextureManager;
@class InputManager;

@interface MapWindowController : NSWindowController {
	IBOutlet MapView2D* view2D;
	IBOutlet MapView3D* view3D;
    IBOutlet TextureView* textureView;
    RenderMap* renderMap;
    Camera* camera;
    TextureManager* textureManager;
    InputManager* inputManager;
}

@end
