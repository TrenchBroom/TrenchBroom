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

@class CompilerProfileCommand;
@class ConsoleWindowController;
@class CompilerProfileRunner;

@interface CompilerProfile : NSObject {
    NSString* name;
    NSMutableArray* commands;
}

- (id)initWithName:(NSString *)theName commands:(NSArray *)theCommands;

- (NSString *)name;
- (void)setName:(NSString *)theName;

- (NSArray *)commands;
- (void)insertObject:(CompilerProfileCommand *)theCommand inCommandsAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromCommandsAtIndex:(NSUInteger)theIndex;

- (CompilerProfileRunner *)runnerWithConsole:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements;

@end
