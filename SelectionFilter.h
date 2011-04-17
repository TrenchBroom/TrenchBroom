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

@interface SelectionFilter : NSObject <Filter> {
    SelectionManager* selectionManager;
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager;

@end
