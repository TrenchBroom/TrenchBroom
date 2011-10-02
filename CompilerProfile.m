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

#import "CompilerProfile.h"
#import "CompilerProfileManager.h"
#import "CompilerProfileCommand.h"
#import "CompilerProfileRunner.h"

@implementation CompilerProfile

- (id)init {
    if ((self = [super init])) {
        commands = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName commands:(NSArray *)theCommands {
    if ((self = [super init])) {
        name = [theName retain];
        commands = [[NSMutableArray alloc] initWithArray:theCommands];
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [commands release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (void)setName:(NSString *)theName {
    [name release];
    name = [theName retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (NSArray *)commands {
    return commands;
}

- (void)insertObject:(CompilerProfileCommand *)theCommand inCommandsAtIndex:(NSUInteger)theIndex {
    [commands insertObject:theCommand atIndex:theIndex];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (void)removeObjectFromCommandsAtIndex:(NSUInteger)theIndex {
    [commands removeObjectAtIndex:theIndex];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (CompilerProfileRunner *)runnerWithConsole:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements {
    return [[[CompilerProfileRunner alloc] initWithProfile:self console:theConsole workDir:theWorkDir replacements:theReplacements] autorelease];
}

@end
