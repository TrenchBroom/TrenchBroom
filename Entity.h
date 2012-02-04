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

@protocol Map;
@class VBOMemBlock;
@class EntityDefinition;
@class PickingHitList;

static NSString* const ClassnameKey = @"classname";
static NSString* const SpawnFlagsKey = @"spawnflags";
static NSString* const WorldspawnClassname = @"worldspawn";
static NSString* const GroupClassName = @"func_group";
static NSString* const GroupNameKey = @"name";
static NSString* const GroupVisibilityKey = @"visible";
static NSString* const OriginKey = @"origin";
static NSString* const AngleKey = @"angle";
static NSString* const MessageKey = @"message";
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

- (void)pick:(const TRay *)theRay hitList:(PickingHitList *)theHitList;

- (VBOMemBlock *)boundsMemBlock;
- (void)setBoundsMemBlock:(VBOMemBlock *)theBoundsMemBlock;

@end
