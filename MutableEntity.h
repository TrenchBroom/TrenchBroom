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

@interface MutableEntity : NSObject <Entity> {
    @private
    NSNumber* entityId;
    id <Map> map;
	NSMutableArray* brushes;
	NSMutableDictionary* properties;
    TVector3f center;
    TBoundingBox bounds;
    BOOL valid;
    NSMutableDictionary* memBlocks;
}

- (id)initWithProperties:(NSDictionary *)theProperties;

- (void)addBrush:(MutableBrush *)brush;
- (void)removeBrush:(MutableBrush *)brush;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;

- (void)setMap:(id <Map>)theMap;
@end
