/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "SpawnFlag.h"


@implementation SpawnFlag

- (id)initWithName:(NSString *)theName flag:(int)theFlag {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theFlag > 0, @"flag must be a positive integer");
    
    if ((self = [self init])) {
        name = [theName retain];
        flag = theFlag;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (int)flag {
    return flag;
}

- (NSComparisonResult)compareByFlag:(SpawnFlag *)otherFlag {
    if (flag > [otherFlag flag])
        return NSOrderedAscending;
    if (flag < [otherFlag flag])
        return NSOrderedDescending;
    return NSOrderedSame;
}
@end
