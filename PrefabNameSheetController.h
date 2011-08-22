//
//  PrefabNameSheetController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PrefabNameSheetController : NSWindowController <NSComboBoxDataSource> {
    IBOutlet NSTextField* prefabNameField;
    IBOutlet NSComboBox* prefabGroupField;
    IBOutlet NSTextField* errorLabel;
    IBOutlet NSButton* cancelButton;
    IBOutlet NSButton* createButton;
}

- (IBAction)createPrefabClicked:(id)sender;
- (IBAction)cancelClicked:(id)sender;

- (NSString *)prefabName;
- (NSString *)prefabGroup;

- (void)setPrefabName:(NSString *)thePrefabName;
- (void)setPrefabGroup:(NSString *)thePrefabGroup;
@end
