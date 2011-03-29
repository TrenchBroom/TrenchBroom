//
//  Entity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Map;
@class BoundingBox;
@class Vector3f;

@protocol Entity <NSObject>

- (NSNumber *)entityId;
- (id <Map>)map;

- (NSArray *)brushes;

- (NSString *)propertyForKey:(NSString *)key;
- (NSString *)classname;

- (NSDictionary *)properties;

- (BOOL)isWorldspawn;

- (BoundingBox *)bounds;
- (Vector3f *)center;

@end
