//
//  RotateCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateCursor.h"


@implementation RotateCursor

- (id)init {
    if ((self = [super init])) {
    }
    
    return self;
}

- (void)dealloc {
    free(quads);
    [super dealloc];
}

@end
