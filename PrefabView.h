//
//  PrefabView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Prefab;
@class GLResources;

@interface PrefabView : NSOpenGLView {
    float gridSize;
    Prefab* draggedPrefab;
    float hAngle, vAngle;
    GLResources* glResources;
}

- (void)setGLResources:(GLResources *)theGLResources;

@end
