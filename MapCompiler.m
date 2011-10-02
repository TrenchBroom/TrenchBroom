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

#import "MapCompiler.h"
#import "ConsoleWindowController.h"
#import "CompilerProfile.h"
#import "CompilerProfileRunner.h"

static NSString* const MapFileReplacement = @"$map";
static NSString* const BspFileReplacement = @"$bsp";

@implementation MapCompiler

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl profile:(CompilerProfile *)theProfile console:(ConsoleWindowController *)theConsole {
    NSAssert(theMapFileUrl != nil, @"map file URL must not be nil");
    NSAssert(theProfile != nil, @"profile must not be nil");
    NSAssert(theConsole != nil, @"console must not be nil");
    
    if ((self = [self init])) {
        NSString* mapFilePath = [theMapFileUrl path];
        NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
        NSString* mapFileName = [mapFilePath lastPathComponent];
        NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
        NSString* bspFileName = [baseFileName stringByAppendingPathExtension:@"bsp"];
        
        NSMutableDictionary* replacements = [[NSMutableDictionary alloc] init];
        [replacements setObject:mapFileName forKey:MapFileReplacement];
        [replacements setObject:bspFileName forKey:BspFileReplacement];
        
        profileRunner = [[theProfile runnerWithConsole:theConsole workDir:mapDirPath replacements:replacements] retain];
        [replacements release];
    }
    
    return self;
}

- (void)dealloc {
    [profileRunner release];
    [super dealloc];
}

- (void)compile {
    [profileRunner run];
}

@end
