//
//  Entity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@protocol Map;
@class VBOMemBlock;
@class EntityDefinition;
@class PickingHitList;

static NSString* const ClassnameKey = @"classname";
static NSString* const SpawnFlagsKey = @"spawnflags";
static NSString* const WorldspawnClassname = @"worldspawn";
static NSString* const OriginKey = @"origin";
static NSString* const AngleKey = @"angle";
static NSString* const ModsKey = @"_mods";

@protocol Entity <NSObject, NSCopying>

- (NSNumber *)entityId;
- (id <Map>)map;
- (id)copy;

- (NSArray *)brushes;

- (NSString *)propertyForKey:(NSString *)key;
- (NSDictionary *)properties;
- (BOOL)isPropertyDeletable:(NSString *)theKey;
- (BOOL)isPropertyWritable:(NSString *)theKey;
- (NSString *)spawnFlagsString;

- (EntityDefinition *)entityDefinition;
- (BOOL)isWorldspawn;
- (NSString *)classname;

- (TBoundingBox *)maxBounds;
- (TBoundingBox *)bounds;
- (TVector3f *)center;
- (TVector3i *)origin;
- (NSNumber *)angle;

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList;

@end
