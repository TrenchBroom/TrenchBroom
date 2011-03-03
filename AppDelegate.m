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
    NSString* resourcePath = [mainBundle resourcePath];
    NSString* prefabPath = [NSString pathWithComponents:[NSArray arrayWithObjects:resourcePath, @"Prefabs", nil]];

    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    [prefabManager loadPrefabsAtPath:prefabPath readOnly:YES];
}

@end
