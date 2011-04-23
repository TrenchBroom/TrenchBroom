//
//  EntityDefinition.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    EDT_POINT,
    EDT_BRUSH,
    EDT_BASE
} EEntityDefinitionType;

@class BoundingBox;

@interface EntityDefinition : NSObject {
    @private
    EEntityDefinitionType type;
    NSString* name;
    float color[3];
    BoundingBox* bounds;
    NSMutableArray* flags;
    NSArray* properties;
    NSString* description;
}

- (id)initBaseDefinitionWithName:(NSString *)theName flags:(NSArray *)theFlags properties:(NSArray *)theProperties;
- (id)initPointDefinitionWithName:(NSString *)theName color:(float *)theColor bounds:(BoundingBox *)theBounds flags:(NSArray *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription;
- (id)initBrushDefinitionWithName:(NSString *)theName color:(float *)theColor flags:(NSArray *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription;

- (EEntityDefinitionType)type;
- (NSString *)name;
- (float *)color;
- (BoundingBox *)bounds;
- (NSArray *)flags;
- (NSArray *)properties;
- (NSString *)description;

- (NSComparisonResult)compareByName:(EntityDefinition *)definition;

@end
