//
//  PrefabView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Camera;
@class Prefab;
@class GLResources;

@interface PrefabView : NSOpenGLView {
    NSMutableDictionary* cameras;
    Prefab* draggedPrefab;
    GLResources* glResources;
    int prefabsPerRow;
}

- (void)setGLResources:(GLResources *)theGLResources;
- (void)setPrefabsPerRow:(int)thePrefabsPerRow;

@end
