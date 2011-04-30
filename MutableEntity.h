//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Entity.h"
#import "Math.h"

@protocol Map;
@class MutableBrush;
@class Face;
@class EntityDefinitionManager;

@interface MutableEntity : NSObject <Entity> {
    @private
    EntityDefinitionManager* definitionManager;
    NSNumber* entityId;
    id <Map> map;
	NSMutableArray* brushes;
	NSMutableDictionary* properties;
    TVector3f center;
    TBoundingBox bounds;
    BOOL valid;
    NSMutableDictionary* memBlocks;
}

- (id)initWithEntityDefinitionManager:(EntityDefinitionManager *)theDefinitionManager;
- (id)initWithProperties:(NSDictionary *)theProperties;
- (id)initWithProperties:(NSDictionary *)theProperties entityDefinitionManager:(EntityDefinitionManager *)theDefinitionManager;

- (void)addBrush:(MutableBrush *)brush;
- (void)removeBrush:(MutableBrush *)brush;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;

- (void)setMap:(id <Map>)theMap;
@end
