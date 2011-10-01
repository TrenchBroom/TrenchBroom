//
//  AppDelegate.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
