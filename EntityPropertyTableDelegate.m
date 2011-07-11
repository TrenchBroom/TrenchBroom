//
//  EntityPropertyTableDelegate.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityPropertyTableDelegate.h"


@implementation EntityPropertyTableDelegate

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
    NSIndexSet* selectedRows = [tableView selectedRowIndexes];
    [removePropertyButton setEnabled:[selectedRows count] > 0];
}

@end
