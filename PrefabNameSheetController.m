//
//  PrefabNameSheetController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabNameSheetController.h"
#import "PrefabManager.h"
#import "PrefabGroup.h"
#import "Prefab.h"

@implementation PrefabNameSheetController

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    return [[prefabManager groups] count];
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    return [[prefabManager groups] objectAtIndex:index];
}

- (NSString *)comboBox:(NSComboBox *)aComboBox completedString:(NSString *)string {
    NSString* lowercaseString = [string lowercaseString];
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    NSEnumerator* groupEn = [[prefabManager groups] objectEnumerator];
    id <PrefabGroup> group;
    while ((group = [groupEn nextObject]))
        if ([[[group name] lowercaseString] hasPrefix:lowercaseString])
            return [group name];
    return nil;
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    NSEnumerator* groupEn = [[prefabManager groups] objectEnumerator];
    id <PrefabGroup> group;
    int index = 0;
    while ((group = [groupEn nextObject])) {
        if ([[group name] isEqualToString:string])
            return index;
        index++;
    }
    return NSNotFound;
}

- (NSString *)windowNibName {
    return @"PrefabNameSheet";
}

- (IBAction)createPrefabClicked:(id)sender {
    NSApplication* app = [NSApplication sharedApplication];
    [self close];
    [app endSheet:[self window] returnCode:NSOKButton];
}

- (IBAction)cancelClicked:(id)sender {
    NSApplication* app = [NSApplication sharedApplication];
    [self close];
    [app endSheet:[self window] returnCode:NSCancelButton];
}

- (NSString *)prefabName {
    return [prefabNameField stringValue];
}
- (NSString *)prefabGroup {
    return [prefabGroupField stringValue];
}

@end
