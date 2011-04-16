//
//  MapView3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class InputManager;
@class Renderer;
@class Options;

@interface MapView3D : NSOpenGLView {
    @private
    float backgroundColor[3];
    Renderer* renderer;
    Options* options;
    NSTrackingArea* mouseTracker;
}

- (void)setup;
- (void)userDefaultsChanged:(NSNotification *)notification;
- (Renderer *)renderer;

@end
