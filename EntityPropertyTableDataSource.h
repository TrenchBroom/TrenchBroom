//
//  EntityPropertyTableDataSource.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapWindowController;

@interface EntityPropertyTableDataSource : NSObject <NSTableViewDataSource> {
    MapWindowController* mapWindowController;
    NSArray* entities;
    NSDictionary* properties;
    NSArray* sortedKeys;
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;
- (void)setEntities:(NSArray *)theEntities;
- (void)updateProperties;
- (NSString *)propertyKeyAtIndex:(NSUInteger)theIndex;
- (BOOL)editingAllowed:(NSTableColumn *)theTableColumn rowIndex:(NSUInteger)theIndex;

@end
