//
//  QuickBarWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CompassView;
@class MapWindowController;

@interface QuickBarWindowController : NSWindowController {
    IBOutlet NSTextField* selectionOriginXField;
    IBOutlet NSTextField* selectionOriginYField;
    IBOutlet NSTextField* selectionOriginZField;
    IBOutlet NSTextField* selectionSizeXField;
    IBOutlet NSTextField* selectionSizeYField;
    IBOutlet NSTextField* selectionSizeZField;
    IBOutlet NSTextField* cameraPosXField;
    IBOutlet NSTextField* cameraPosYField;
    IBOutlet NSTextField* cameraPosZField;
    IBOutlet NSTextField* cameraDirXField;
    IBOutlet NSTextField* cameraDirYField;
    IBOutlet NSTextField* cameraDirZField;
    IBOutlet CompassView* compassView;
    MapWindowController* mapWindowController;
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;

@end
