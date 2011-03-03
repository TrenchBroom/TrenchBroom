//
//  PrefabLayoutGroupRow.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol PrefabGroup;
@class GLFont;

@interface PrefabLayoutGroupRow : NSObject {
    id <PrefabGroup> prefabGroup;
    NSRect nameBounds;
    NSRect bounds;
    NSMutableArray* cells;
}

- (id)initWithPrefabGroup:(id <PrefabGroup>)thePrefabGroup prefabsPerRow:(int)prefabsPerRow glFont:(GLFont *)theGLFont atPos:(NSPoint)thePos width:(float)theWidth innerMargin:(float)innerMargin;

- (id <PrefabGroup>)prefabGroup;
- (NSArray *)cells;

- (NSRect)nameBounds;
- (NSRect)bounds;

@end
