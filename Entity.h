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

@protocol Entity <NSObject>

- (NSNumber *)entityId;
- (id <Map>)map;

- (NSArray *)brushes;

- (NSString *)propertyForKey:(NSString *)key;
- (NSDictionary *)properties;

- (EntityDefinition *)entityDefinition;
- (BOOL)isWorldspawn;
- (NSString *)classname;

- (TBoundingBox *)bounds;
- (TVector3f *)center;

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList;

- (void)setMemBlock:(VBOMemBlock *)theBlock forKey:(id <NSCopying>)theKey;
- (VBOMemBlock *)memBlockForKey:(id <NSCopying>)theKey;

@end
