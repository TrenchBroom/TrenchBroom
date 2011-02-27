//
//  Prefab.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Map.h"

@protocol Map;
@class Entity;

@interface Prefab : NSObject <Map> {
    Entity* entity;
}

- (void)translateToOrigin;

@end
