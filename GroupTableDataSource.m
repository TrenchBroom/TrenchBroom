//
//  GroupTableDataSource.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GroupTableDataSource.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "Entity.h"
#import "GroupManager.h"

@implementation GroupTableDataSource

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    if (mapWindowController == theMapWindowController)
        return;
    
    mapWindowController = theMapWindowController;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
    if (mapWindowController == nil)
        return 0;
    
    MapDocument* map = [mapWindowController document];
    GroupManager* groupManager = [map groupManager];
    NSArray* groups = [groupManager groups];
    
    return [groups count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    if (mapWindowController == nil)
        return nil;
    
    MapDocument* map = [mapWindowController document];
    GroupManager* groupManager = [map groupManager];
    NSArray* groups = [groupManager groups];
    
    if (rowIndex < 0 || rowIndex >= [groups count])
        return nil;
    
    id <Entity> group = [groups objectAtIndex:rowIndex];
    if ([@"Name" isEqualToString:[aTableColumn identifier]]) {
        NSString* name = [group propertyForKey:GroupNameKey];
        if (name == nil)
            return @"unnamed";
        return name;
    } else if ([@"Visible" isEqualToString:[aTableColumn identifier]]) {
        return [NSNumber numberWithBool:[groupManager isVisible:group]];
    }
    
    return nil;
}    

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    if (mapWindowController == nil)
        return;

    MapDocument* map = [mapWindowController document];
    GroupManager* groupManager = [map groupManager];
    NSArray* groups = [groupManager groups];
    
    if (rowIndex < 0 || rowIndex >= [groups count])
        return;
    
    if ([@"Name" isEqualToString:[aTableColumn identifier]]) {
        NSString* name = (NSString *)anObject;
        id <Entity> group = [groups objectAtIndex:rowIndex];
        [groupManager setGroup:group name:name];
    } else if ([@"Visible" isEqualToString:[aTableColumn identifier]]) {
        NSNumber* visible = (NSNumber *)anObject;
        id <Entity> group = [groups objectAtIndex:rowIndex];
        [groupManager setGroup:group visibility:[visible boolValue]];
    }
}

@end
