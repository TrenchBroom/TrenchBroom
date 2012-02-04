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

#import "DefaultProperty.h"


@implementation DefaultProperty

- (id)initWithName:(NSString *)theName value:(NSString *)theValue {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theValue != nil, @"value must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        value = [theValue retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_DEFAULT;
}

- (NSString *)name {
    return name;
}

- (NSString *)value {
    return value;
}

- (void)dealloc {
    [name release];
    [value release];
    [super dealloc];
}

@end
