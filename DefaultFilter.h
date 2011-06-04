//
//  SelectionFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Filter.h"

@class SelectionManager;
@class Options;

@interface DefaultFilter : NSObject <Filter> {
    SelectionManager* selectionManager;
    Options* options;
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager options:(Options *)theOptions;

@end
