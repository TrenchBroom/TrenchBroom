//
//  SelectionTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"

@class MapWindowController;

@interface SelectionTool : DefaultTool {
    MapWindowController* windowController;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
