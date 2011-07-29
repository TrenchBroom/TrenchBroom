//
//  InspectorWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class InspectorViewController;
@class MapWindowController;

@interface InspectorWindowController : NSWindowController {
    InspectorViewController* inspectorViewController;
    MapWindowController* mapWindowController;
}

+ (InspectorWindowController *)sharedInspector;

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;
- (MapWindowController *)mapWindowController;

@end
