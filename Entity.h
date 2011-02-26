//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;
@class MapDocument;
@class Brush;
@class Face;

@interface Entity : NSObject {
    @private
    MapDocument* map;
    NSNumber* entityId;
	NSMutableArray* brushes;
    NSMutableDictionary* brushIndices;
	NSMutableDictionary* properties;
}

- (id)initInMap:(MapDocument *)theMap;
- (id)initInMap:(MapDocument *)theMap property:(NSString *)key value:(NSString *)value;

- (Brush *)createBrush;
- (void)addBrush:(Brush *)brush;
- (void)removeBrush:(Brush *)brush;

- (MapDocument *)map;
- (NSNumber *)entityId;

- (NSArray *)brushes;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;
- (NSString *)propertyForKey:(NSString *)key;
- (NSString *)classname;

- (NSDictionary *)properties;

- (BOOL)isWorldspawn;

- (NSUndoManager *)undoManager;

- (void)faceFlagsChanged:(Face *)face;
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture;
- (void)faceGeometryChanged:(Face *)face;

- (void)faceAdded:(Face *)face;
- (void)faceRemoved:(Face *)face;
@end
