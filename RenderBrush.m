//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderBrush.h"


@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        polygons = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)aBrush {
    if (self = [self init]) {
        [self setBrush:aBrush];
    }
    
    return self;
}

- (void)brushChanged:(NSNotification *)notification {
}

- (Brush *)brush {
    
    return brush;
}

- (void)setBrush:(Brush *)aBrush {
    NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
    if (brush) {
        [notificationCenter removeObserver:self name:BrushFaceAdded object:brush];
        [notificationCenter removeObserver:self name:BrushFaceRemoved object:brush];
        [brush release];
    }
    
    brush = [aBrush retain];
    
    [notificationCenter addObserver:self selector:@selector(brushChanged:) name:BrushFaceAdded object:brush];
    [notificationCenter addObserver:self selector:@selector(brushChanged:) name:BrushFaceRemoved object:brush];
}

- (void)dealloc {
    [brush release];
    [polygons release];

    [super dealloc];
}

@end
