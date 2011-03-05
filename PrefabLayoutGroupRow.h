//
//  PrefabLayoutGroupRow.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol PrefabGroup;
@protocol Prefab;
@class GLFont;

@interface PrefabLayoutGroupRow : NSObject {
    id <PrefabGroup> prefabGroup;
    NSRect titleBarBounds;
    NSRect titleBounds;
    NSRect bounds;
    NSMutableArray* cells;
}

- (id)initWithPrefabGroup:(id <PrefabGroup>)thePrefabGroup prefabsPerRow:(int)prefabsPerRow glFont:(GLFont *)theGLFont atPos:(NSPoint)thePos width:(float)theWidth innerMargin:(float)innerMargin;

- (id <PrefabGroup>)prefabGroup;
- (NSArray *)cells;

- (id <Prefab>)prefabAt:(NSPoint)pos;

- (NSRect)titleBarBounds;
- (NSRect)titleBounds;
- (NSRect)bounds;

@end
