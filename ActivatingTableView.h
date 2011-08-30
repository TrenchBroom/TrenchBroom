//
//  NSActivatingTableView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//



@interface ActivatingTableView : NSTableView {
    IBOutlet NSView* contentView;
    IBOutlet NSView* detailView;
}

@end
