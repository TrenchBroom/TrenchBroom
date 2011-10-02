/*
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

#import "ChoiceProperty.h"


@implementation ChoiceProperty

- (id)initWithName:(NSString*)theName arguments:(NSArray *)theArguments {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theArguments != nil, @"argument array must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        arguments = [theArguments retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_CHOICE;
}

- (NSString *)name {
    return name;
}

- (NSArray *)arguments {
    return arguments;
}

- (void)dealloc {
    [name release];
    [arguments release];
    [super dealloc];
}

@end
