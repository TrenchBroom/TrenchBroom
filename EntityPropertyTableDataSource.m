/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "EntityPropertyTableDataSource.h"
#import "Entity.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "EntityDefinition.h"
#import "SpawnFlag.h"
#import "Entity.h"

@implementation EntityPropertyTableDataSource

- (void)dealloc {
    [entities release];
    [properties release];
    [sortedKeys release];
    [super dealloc];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    mapWindowController = theMapWindowController;
}

- (void)setEntities:(NSArray *)theEntities {
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
        NSString* spawnFlagsString = [entity spawnFlagsString];
        [mergedProperties setObject:spawnFlagsString forKey:SpawnFlagsKey];
        
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
                    NSString* newValue;
                    if (![SpawnFlagsKey isEqualToString:key])
                        newValue = [[entity properties] objectForKey:key];
                    else
                        newValue = [entity spawnFlagsString];
                    
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

- (NSString *)propertyKeyAtIndex:(NSUInteger)theIndex {
    if (properties == nil)
        return nil;
    
    return [sortedKeys objectAtIndex:theIndex];
}

- (NSUInteger)indexOfPropertyWithKey:(NSString *)theKey {
    return [sortedKeys indexOfObject:theKey];
}

- (BOOL)editingAllowed:(NSTableColumn *)theTableColumn rowIndex:(NSUInteger)theIndex {
    if (properties == nil)
        return NO;
    
    if (theIndex >= [properties count])
        return NO;
    
    if ([@"Key" isEqualToString:[theTableColumn identifier]]) {
        NSString* key = [sortedKeys objectAtIndex:theIndex];
        
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            if (![entity isPropertyDeletable:key])
                return NO;
    } else if ([@"Value" isEqualToString:[theTableColumn identifier]]) {
        NSString* key = [sortedKeys objectAtIndex:theIndex];
        
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            if (![entity isPropertyWritable:key])
                return NO;
    }
    
    return YES;
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
        
        [map setEntities:entities propertyKey:key value:newValue];
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Change Property Value"];
    }
}


@end
