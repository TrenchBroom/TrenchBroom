//
//  AppDelegate.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "AppDelegate.h"
#import "PrefabManager.h"

@implementation AppDelegate

+ (void)initialize {
    NSString* defaultsPath = [[NSBundle mainBundle] pathForResource:@"Defaults" ofType:@"plist"];
    NSDictionary* defaults = [NSDictionary dictionaryWithContentsOfFile:defaultsPath];
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    srand(time(NULL));
    
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* bundlePath = [mainBundle bundlePath];

    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    [prefabManager loadPrefabsAtPath:bundlePath];
}

@end
