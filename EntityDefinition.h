/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    EDT_POINT,
    EDT_BRUSH,
    EDT_BASE
} EEntityDefinitionType;

static NSString* const EntityDefinitionType = @"EntityDefinition";

@class SpawnFlag;
@class ModelProperty;
@protocol Entity;

@interface EntityDefinition : NSObject {
    @private
    EEntityDefinitionType type;
    NSString* name;
    TVector4f color;
    TVector3f center;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    NSDictionary* flags;
    NSArray* properties;
    NSString* description;
    int usageCount;
}

- (id)initBaseDefinitionWithName:(NSString *)theName flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties;
- (id)initPointDefinitionWithName:(NSString *)theName color:(TVector4f *)theColor bounds:(TBoundingBox *)theBounds flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription;
- (id)initBrushDefinitionWithName:(NSString *)theName color:(TVector4f *)theColor flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription;

- (EEntityDefinitionType)type;
- (NSString *)name;
- (const TVector4f *)color;
- (const TVector3f *)center;
- (const TBoundingBox *)bounds;
- (const TBoundingBox *)maxBounds;
- (SpawnFlag *)flagForName:(NSString *)theName;
- (NSArray *)flagsForMask:(int)theMask;
- (NSArray *)allFlags;
- (BOOL)isFlag:(NSString *)theFlagName setOnEntity:(id <Entity>)theEntity;
- (NSArray *)properties;
- (ModelProperty *)modelPropertyForEntity:(id <Entity>)theEntity;
- (ModelProperty *)defaultModelProperty;
- (NSString *)description;

- (void)incUsageCount;
- (void)decUsageCount;
- (int)usageCount;

- (NSComparisonResult)compareByName:(EntityDefinition *)definition;
- (NSComparisonResult)compareByUsageCount:(EntityDefinition *)definition;

@end
