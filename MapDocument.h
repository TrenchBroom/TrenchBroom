//
//  MyDocument.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//


#import <Cocoa/Cocoa.h>

@class Map;
@class Picker;
@class GLResources;

@interface MapDocument : NSDocument {
    Map* map;
    Picker* picker;
    GLResources* glResources;
}

- (Map *)map;
- (Picker *)picker;
- (GLResources *)glResources;

@end
