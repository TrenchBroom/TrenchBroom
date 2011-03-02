//
//  PrefabManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const PrefabAdded;
extern NSString* const PrefabRemoved;
extern NSString* const PrefabKey;

@interface PrefabManager : NSObject {
    @private
    NSMutableDictionary* prefabs;
}

+ (PrefabManager *)sharedPrefabManager;

- (void)loadPrefabsAtPath:(NSString *)thePath;
- (void)loadPrefab:(NSData *)prefabData name:(NSString *)prefabName;

- (NSArray *)prefabs;

@end
