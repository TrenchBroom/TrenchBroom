//
//  CursorManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CursorManager.h"
#import "Cursor.h"
#import "PickingHit.h"

NSString* const CursorChanged = @"CursorChanged";

@implementation CursorManager

- (id)init {
    if ((self = [super init])) {
        cursorStack = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)pushCursor:(id <Cursor>)cursor {
    NSAssert(cursor != nil, @"cursor must not be nil");
    [cursorStack addObject:cursor];
//    [NSCursor hide];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:CursorChanged object:self];
}

- (void)popCursor {
    if ([cursorStack count] == 0)
        [NSException raise:NSInternalInconsistencyException format:@"cannot pop from empty cursor stack"];
    
    [cursorStack removeLastObject];
//    if ([cursorStack count] == 0)
//        [NSCursor unhide];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:CursorChanged object:self];
}

- (BOOL)empty {
    return [cursorStack count] == 0;
}

- (void)render {
    if ([cursorStack count] > 0) {
        id <Cursor> cursor = [cursorStack lastObject];
        [cursor render];
    }
}

- (void)dealloc {
    [cursorStack release];
    [super dealloc];
}

@end
