//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;
@class Vector3f;
@class Brush;
@class Face;
@class BoundingBox;
@protocol Map;

@interface Entity : NSObject {
    @private
    id<Map> map;
    NSNumber* entityId;
	NSMutableArray* brushes;
    NSMutableDictionary* brushIndices;
	NSMutableDictionary* properties;
    Vector3f* center;
    BoundingBox* bounds;
}

- (id)initInMap:(id<Map>)theMap;
- (id)initInMap:(id<Map>)theMap property:(NSString *)key value:(NSString *)value;

- (Brush *)createBrush;
- (Brush *)createBrushFromTemplate:(Brush *)theTemplate;
- (void)addBrush:(Brush *)brush;
- (void)removeBrush:(Brush *)brush;

- (id<Map>)map;
- (NSNumber *)entityId;

- (NSArray *)brushes;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;
- (NSString *)propertyForKey:(NSString *)key;
- (NSString *)classname;

- (NSDictionary *)properties;

- (BOOL)isWorldspawn;

- (BoundingBox *)bounds;
- (Vector3f *)center;

- (void)faceFlagsChanged:(Face *)face;
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture;
- (void)faceGeometryChanged:(Face *)face;

- (void)faceAdded:(Face *)face;
- (void)faceRemoved:(Face *)face;
@end
