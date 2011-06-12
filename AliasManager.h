//
//  AliasManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Alias;

@interface AliasManager : NSObject {
    NSMutableDictionary* aliases;
}

+ (AliasManager *)sharedManager;

- (Alias *)aliasWithName:(NSString *)theName paths:(NSArray *)thePaths;

@end
