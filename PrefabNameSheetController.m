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
    return [[prefabManager prefabGroups] count];
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    return [[prefabManager prefabGroups] objectAtIndex:index];
}

- (NSString *)comboBox:(NSComboBox *)aComboBox completedString:(NSString *)string {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    return [[prefabManager prefabGroupWithNamePrefix:string] name];
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string {
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    return [prefabManager indexOfPrefabGroupWithName:string];
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

- (void)validate {
    [prefabNameField setTextColor:[NSColor textColor]];
    [prefabGroupField setTextColor:[NSColor textColor]];
    [errorLabel setStringValue:@""];
    [createButton setEnabled:YES];
    
    NSString* prefabName = [self prefabName];
    NSString* prefabGroupName = [self prefabGroup];
    
    if ([prefabName length] == 0) {
        [prefabNameField setTextColor:[NSColor redColor]];
        [errorLabel setStringValue:@"Enter a prefab name."];
        [createButton setEnabled:NO];
        return;
    }
    
    if ([prefabGroupName length] == 0) {
        [prefabGroupField setTextColor:[NSColor redColor]];
        [errorLabel setStringValue:@"Enter a prefab group name."];
        [createButton setEnabled:NO];
        return;
    }
    
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    id <PrefabGroup> group = [prefabManager prefabGroupWithName:prefabGroupName create:NO];
    if (group != nil) {
        id <Prefab> prefab = [group prefabWithName:prefabName];
        if (prefab != nil) {
            [prefabNameField setTextColor:[NSColor redColor]];
            [errorLabel setStringValue:@"A prefab with this name exists."];
            [createButton setEnabled:NO];
            return;
        }
    }
}

- (void)controlTextDidChange:(NSNotification *)notification {
    [self validate];
}

- (void)comboBoxSelectionDidChange:(NSNotification *)notification {
    NSInteger index = [prefabGroupField indexOfSelectedItem];
    if (index != -1) {
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        id <PrefabGroup> prefabGroup = [[prefabManager prefabGroups] objectAtIndex:index];
        [prefabGroupField setStringValue:[prefabGroup name]];
    }
    [self validate];
}

- (NSString *)prefabName {
    return [[prefabNameField stringValue] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
}

- (NSString *)prefabGroup {
    return [[prefabGroupField stringValue] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
}

@end
