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
@class EntityDefinition;
@class VBOMemBlock;

@interface MutableEntity : NSObject <Entity> {
    EntityDefinition* entityDefinition;
    NSNumber* entityId;
    id <Map> map;
	NSMutableArray* brushes;
	NSMutableDictionary* properties;
    TVector3f center;
    TVector3i origin;
    NSNumber* angle;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    BOOL valid;
    int filePosition;
    
    VBOMemBlock* boundsMemBlock;
}

- (id)initWithProperties:(NSDictionary *)theProperties;

- (void)addBrush:(MutableBrush *)brush;
- (void)removeBrush:(MutableBrush *)brush;
- (void)brushChanged:(MutableBrush *)brush;

- (void)translateBy:(const TVector3i *)theDelta;
- (void)rotateZ90CW:(const TVector3i *)theRotationCenter;
- (void)rotateZ90CCW:(const TVector3i *)theRotationCenter;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theRotationCenter;

- (void)replaceProperties:(NSDictionary *)theProperties;
- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;

- (void)setEntityDefinition:(EntityDefinition *)theEntityDefintion;
- (void)setMap:(id <Map>)theMap;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end
