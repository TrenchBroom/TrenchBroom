//
//  CameraTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"

@class MapWindowController;
@class Vector3f;

@interface CameraTool : DefaultTool {
    MapWindowController* windowController;
    Vector3f* orbitCenter;
    BOOL gesture;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
