/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "CursorManager.h"
#import "Cursor.h"
#import "PickingHit.h"

NSString* const CursorChanged = @"CursorChanged";

@implementation CursorManager

- (id)initWithCamera:(Camera *)theCamera {
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    if ((self = [self init])) {
        camera = theCamera;
        cursorStack = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)pushCursor:(id <Cursor>)cursor {
    NSAssert(cursor != nil, @"cursor must not be nil");
    [cursorStack addObject:cursor];
    [NSCursor hide];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:CursorChanged object:self];
}

- (void)popCursor {
    if ([cursorStack count] == 0)
        [NSException raise:NSInternalInconsistencyException format:@"cannot pop from empty cursor stack"];
    
    [cursorStack removeLastObject];
    if ([cursorStack count] == 0)
        [NSCursor unhide];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:CursorChanged object:self];
}

- (BOOL)empty {
    return [cursorStack count] == 0;
}

- (void)render {
    if ([cursorStack count] > 0) {
        id <Cursor> cursor = [cursorStack lastObject];
        [cursor render:camera];
    }
}

- (void)dealloc {
    [cursorStack release];
    [super dealloc];
}

@end
