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

#import "CompilerProfileCommand.h"
#import "CompilerProfileManager.h"

@implementation CompilerProfileCommand

- (id)initWithPath:(NSString *)thePath arguments:(NSString *)theArguments {
    if ((self = [self init])) {
        path = [thePath retain];
        arguments = [theArguments retain];
    }
    
    return self;
}

- (void)dealloc {
    [path release];
    [arguments release];
    [super dealloc];
}

- (NSString *)path {
    return path;
}

- (void)setPath:(NSString *)thePath {
    [path release];
    path = [thePath retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (NSString *)arguments {
    return arguments;
}

- (void)setArguments:(NSString *)theArguments {
    [arguments release];
    arguments = [theArguments retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

@end
