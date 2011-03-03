//
//  PrefabNameSheetController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabNameSheetController.h"


@implementation PrefabNameSheetController

- (NSString *)windowNibName {
    return @"PrefabNameSheet";
}

- (IBAction)createPrefabClicked:(id)sender {
    NSApplication* app = [NSApplication sharedApplication];
    [app endSheet:[self window] returnCode:NSOKButton];
}

- (IBAction)cancelClicked:(id)sender {
    NSApplication* app = [NSApplication sharedApplication];
    [app endSheet:[self window] returnCode:NSCancelButton];
}

- (NSString *)prefabName {
    return [prefabNameField stringValue];
}
- (NSString *)prefabGroup {
    return [prefabGroupField stringValue];
}

@end
