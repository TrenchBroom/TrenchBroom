//
//  EntityPropertyTableDataSource.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityPropertyTableDataSource.h"
#import "Entity.h"
#import "MapWindowController.h"
#import "MapDocument.h"

@implementation EntityPropertyTableDataSource

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    [mapWindowController release];
    mapWindowController = [theMapWindowController retain];
}

- (void)setEntities:(NSSet *)theEntities {
    [entities release];
    [properties release];
    properties = nil;
    [sortedKeys release];
    sortedKeys = nil;
    
    entities = [theEntities retain];
    [self updateProperties];
}

- (void)updateProperties {
    if (entities == nil || [entities count] == 0) {
        properties = [[NSDictionary dictionary] retain];
        sortedKeys = [[NSArray array] retain];
    } else {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity = [entityEn nextObject];
        NSMutableDictionary* mergedProperties = [[NSMutableDictionary alloc] initWithDictionary:[entity properties]];
        
        while ((entity = [entityEn nextObject])) {
            NSSet* allKeys = [[NSSet alloc] initWithArray:[mergedProperties allKeys]];
            NSMutableSet* keys = [[NSMutableSet alloc] initWithArray:[[entity properties] allKeys]];
            NSMutableSet* toRemove = [[NSMutableSet alloc] initWithSet:allKeys];
            [toRemove minusSet:keys];
            [keys intersectSet:allKeys];
            
            [mergedProperties removeObjectsForKeys:[toRemove allObjects]];
            
            NSEnumerator* keyEn = [keys objectEnumerator];
            NSString* key;
            while ((key = [keyEn nextObject])) {
                id oldValue = [mergedProperties objectForKey:key];
                if (oldValue != NSMultipleValuesMarker) {
                    NSString* newValue = [[entity properties] objectForKey:key];
                    if (![newValue isEqualToString:(NSString *)oldValue])
                        [mergedProperties setObject:NSMultipleValuesMarker forKey:key];
                }
            }
            
            [allKeys release];
            [keys release];
            [toRemove release];
        }

        properties = mergedProperties;
        sortedKeys = [[[properties allKeys] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)] retain];
    }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
    if (properties == nil)
        return 0;
    return [properties count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    if (properties == nil)
        return nil;

    if (rowIndex < 0 || rowIndex >= [properties count])
        return nil;
    
    if ([@"Key" isEqualToString:[aTableColumn identifier]]) {
        return [sortedKeys objectAtIndex:rowIndex];
    } else if ([@"Value" isEqualToString:[aTableColumn identifier]]) {
        id value = [properties objectForKey:[sortedKeys objectAtIndex:rowIndex]];
        if (value == NSMultipleValuesMarker)
            return @"<multiple>";
        return value;
    }
    
    return nil;
}             

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    if (properties == nil)
        return;
    
    if (rowIndex < 0 || rowIndex >= [properties count])
        return;
    
    if ([@"Key" isEqualToString:[aTableColumn identifier]]) {
        MapDocument* map = [mapWindowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        NSString* oldKey = [sortedKeys objectAtIndex:rowIndex];
        NSString* newKey = (NSString *)anObject;
        
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            NSString* value = [entity propertyForKey:oldKey];
            [map setEntity:entity propertyKey:oldKey value:nil];
            [map setEntity:entity propertyKey:newKey value:value];
        }
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Change Property Key"];
    } else if ([@"Value" isEqualToString:[aTableColumn identifier]]) {
        MapDocument* map = [mapWindowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        NSString* key = [sortedKeys objectAtIndex:rowIndex];
        NSString* newValue = (NSString *)anObject;
        
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            [map setEntity:entity propertyKey:key value:newValue];
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Change Property Value"];
    }
}

- (void)dealloc {
    [mapWindowController release];
    [entities release];
    [properties release];
    [sortedKeys release];
    [super dealloc];
}

@end
