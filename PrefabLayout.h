//
//  PrefabLayout.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PrefabManager;
@class GLFont;
@protocol Prefab;

@interface PrefabLayout : NSObject {
    @private
    NSMutableArray* groupRows;
    PrefabManager* prefabManager;
    GLFont* glFont;
    int prefabsPerRow;
    float outerMargin;
    float innerMargin;
    float groupMargin;
    float width;
    float height;
    BOOL valid;
}

- (id)initWithPrefabManager:(PrefabManager *)thePrefabManager prefabsPerRow:(int)thePrefabsPerRow glFont:(GLFont *)theGLFont;

- (GLFont *)glFont;
- (NSArray *)groupRows;
- (float)height;

- (id <Prefab>)prefabAt:(NSPoint)pos;

- (void)setPrefabsPerRow:(int)thePrefabsPerRow;
- (void)setWidth:(float)width;
@end
