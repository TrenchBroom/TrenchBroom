//
//  PrefabNameSheetController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PrefabNameSheetController : NSWindowController {
    IBOutlet NSTextField* prefabNameField;
    IBOutlet NSTextField* prefabGroupField;
}

- (IBAction)createPrefabClicked:(id)sender;
- (IBAction)cancelClicked:(id)sender;

- (NSString *)prefabName;
- (NSString *)prefabGroup;

@end
