//
//  PrefabView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Prefab;
@class Camera;
@class GLResources;
@class PrefabLayout;

@interface PrefabView : NSOpenGLView {
    NSMutableDictionary* cameras;
    id <Prefab> draggedPrefab;
    GLResources* glResources;
    PrefabLayout* layout;
    int prefabsPerRow;
    NSMutableDictionary* glStrings;
}

- (void)setGLResources:(GLResources *)theGLResources;
- (void)setPrefabsPerRow:(int)thePrefabsPerRow;

@end
