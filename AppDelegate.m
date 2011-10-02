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

#import "AppDelegate.h"
#import "PrefabManager.h"
#import "NSFileManager+AppSupportCategory.h"
#import "PreferencesManager.h"

@implementation AppDelegate

+ (void)initialize {
    NSString* defaultsPath = [[NSBundle mainBundle] pathForResource:@"Defaults" ofType:@"plist"];
    NSDictionary* defaults = [NSDictionary dictionaryWithContentsOfFile:defaultsPath];
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    srand(time(NULL));
    
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* resourcePath = [mainBundle resourcePath];
    NSString* builtinPrefabPath = [NSString pathWithComponents:[NSArray arrayWithObjects:resourcePath, @"Prefabs", nil]];

    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    [prefabManager loadPrefabsAtPath:builtinPrefabPath readOnly:YES];

    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString* appSupportPath = [fileManager findApplicationSupportFolder];
    
    NSString* userPrefabPath = [NSString pathWithComponents:[NSArray arrayWithObjects:appSupportPath, @"Prefabs", nil]];
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:userPrefabPath isDirectory:&directory];
    if (exists && directory)
        [prefabManager loadPrefabsAtPath:userPrefabPath readOnly:NO];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
    return NO;
}

- (PreferencesManager *)preferences {
    return [PreferencesManager sharedManager];
}

@end
