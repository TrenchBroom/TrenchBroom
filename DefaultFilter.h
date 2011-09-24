//
//  SelectionFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Filter.h"

@class GroupManager;
@class SelectionManager;
@class Options;

@interface DefaultFilter : NSObject <Filter> {
    GroupManager* groupManager;
    SelectionManager* selectionManager;
    Options* options;
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager groupManager:(GroupManager *)theGroupManager options:(Options *)theOptions;

@end
