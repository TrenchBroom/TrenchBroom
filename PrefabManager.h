//
//  PrefabManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PrefabManager : NSObject {
    @private
    NSMutableDictionary* prefabs;
}

- (void)loadPrefab:(NSData *)prefabData;

@end
