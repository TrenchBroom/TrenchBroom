//
//  RenderContext.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Camera;

@interface RenderContext : NSObject {
    @private
    Camera* camera;
}

- (Camera *)camera;
- (void)setCamera:(Camera *)aCamera;

@end
