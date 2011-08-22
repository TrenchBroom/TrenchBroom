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
@protocol PrefabViewTarget;

@interface PrefabView : NSOpenGLView {
    NSMutableDictionary* cameras;
    id <Prefab> draggedPrefab;
    id <Prefab> selectedPrefab;
    GLResources* glResources;
    PrefabLayout* layout;
    int prefabsPerRow;
    IBOutlet id <PrefabViewTarget> target;
}

- (void)setGLResources:(GLResources *)theGLResources;
- (void)setPrefabsPerRow:(int)thePrefabsPerRow;

- (IBAction)insertPrefabIntoMap:(id)sender;
- (IBAction)renamePrefab:(id)sender;
- (IBAction)deletePrefab:(id)sender;

@end
