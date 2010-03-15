//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Entity.h"

@interface Map : NSObject {
    Entity* worldspawn;
    NSMutableSet* entities;
}

@end
