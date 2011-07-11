//
//  EntityPropertyTableDelegate.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface EntityPropertyTableDelegate : NSObject <NSTableViewDelegate> {
@private
    IBOutlet NSTableView* tableView;
    IBOutlet NSButton* removePropertyButton;
    IBOutlet NSButton* addPropertyButton;
}

@end
