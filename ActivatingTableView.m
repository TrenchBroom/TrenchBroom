//
//  NSActivatingTableView.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ActivatingTableView.h"

@implementation ActivatingTableView

- (BOOL)becomeFirstResponder {
    if ([super becomeFirstResponder]) {
        NSView* oldView = [[contentView subviews] count] > 0 ? [[contentView subviews] objectAtIndex:0] : nil;
        
        [contentView addSubview:detailView];
        [oldView removeFromSuperview];
        [contentView setNeedsDisplay:YES];
        
        return YES;
    }
    
    return NO;
}

@end
