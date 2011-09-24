//
//  GroupTableDataSource.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class MapWindowController;

@interface GroupTableDataSource : NSObject <NSTableViewDataSource> {
    MapWindowController* mapWindowController;
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;

@end
