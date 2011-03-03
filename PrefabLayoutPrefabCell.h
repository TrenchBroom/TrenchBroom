//
//  PrefabLayoutPrefabCell.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Prefab;
@class GLFont;

@interface PrefabLayoutPrefabCell : NSObject {
    @private
    id <Prefab> prefab;
    NSRect prefabBounds;
    NSRect nameBounds;
    NSRect bounds;
}

- (id)initWithPrefab:(id <Prefab>)thePrefab glFont:(GLFont *)theGLFont atPos:(NSPoint)thePos width:(float)theWidth; 

- (id <Prefab>)prefab;
- (NSRect)prefabBounds;
- (NSRect)nameBounds;
- (NSRect)bounds;

@end
