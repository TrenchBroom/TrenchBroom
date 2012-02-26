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

#import "MutableEntity.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "IdGenerator.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "EntityDefinition.h"
#import "EntityDefinitionManager.h"

@interface MutableEntity (private)

- (void)validate;

@end

@implementation MutableEntity (private)

- (void)validate {
    if (entityDefinition == nil || [entityDefinition type] == EDT_BRUSH) {
        if ([brushes count] > 0) {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            MutableBrush* brush = [brushEn nextObject];
            
            bounds = *[brush bounds];
            for (brush in brushEn)
                mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
            
            centerOfBounds(&bounds, &center);

            TVector3f diff;
            subV3f(&bounds.max, &center, &diff);
            float dist = lengthV3f(&diff);
            diff.x = dist;
            diff.y = dist;
            diff.z = dist;
            subV3f(&center, &diff, &maxBounds.min);
            addV3f(&center, &diff, &maxBounds.max);
        } else {
            bounds.min = NullVector;
            bounds.max = NullVector;
            center = NullVector;
            maxBounds.min = NullVector;
            maxBounds.max = NullVector;
        }
    } else if ([entityDefinition type] == EDT_POINT) {
        bounds = *[entityDefinition bounds];
        
        addV3f(&bounds.min, &origin, &bounds.min);
        addV3f(&bounds.max, &origin, &bounds.max);
        centerOfBounds(&bounds, &center);

        TVector3f diff;
        subV3f(&bounds.max, &center, &diff);
        float dist = lengthV3f(&diff);
        diff.x = dist;
        diff.y = dist;
        diff.z = dist;
        subV3f(&center, &diff, &maxBounds.min);
        addV3f(&center, &diff, &maxBounds.max);
    } else {
        bounds.min = NullVector;
        bounds.max = NullVector;
        center = NullVector;
        maxBounds.min = NullVector;
        maxBounds.max = NullVector;
    }
    
    valid = YES;
}

@end

@implementation MutableEntity

- (id)init {
    if ((self = [super init])) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        filePosition = -1;
        boundsVboBlock = NULL;
        entityDefinition = nil;
    }
    
    return self;
}

- (id)initWithProperties:(NSDictionary *)theProperties {
    if ((self = [self init])) {
        for (NSString* key in theProperties) {
            NSString* value = [theProperties objectForKey:key];
            [self setProperty:key value:value];
        }
    }
    
    return self;
}


- (void) dealloc {
    if (boundsVboBlock != NULL)
        freeVboBlock(boundsVboBlock);
    [entityId release];
	[properties release];
	[brushes release];
    [entityDefinition decUsageCount];
    [entityDefinition release];
    [angle release];
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone {
    MutableEntity* result = [[MutableEntity allocWithZone:zone] initWithProperties:properties];
    [result->entityId release];
    result->entityId = [entityId retain];
    
    if (entityDefinition != nil)
        [result setEntityDefinition:entityDefinition];
    [result setMap:map];
    [result setFilePosition:filePosition];
    
    for (id <Brush> brush in brushes) {
        id <Brush> brushCopy = (id <Brush>) [brush copy];
        [result addBrush:brushCopy];
        [brushCopy release];
    }
    
    return result;
}

- (void)addBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot add brush to point or base entity"];
    
    
    [brushes addObject:brush];
    [brush setEntity:self];
    valid = NO;
}

- (void)removeBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot remove brush from point or base entity"];
    
    [brush setEntity:nil];
    [brushes removeObject:brush];
    valid = NO;
}

- (void)brushChanged:(MutableBrush *)brush {
    valid = NO;
}

- (void)translateBy:(const TVector3f *)theDelta {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];
    
    TVector3f temp;
    addV3f([self origin], theDelta, &temp);
    roundV3f(&temp, &temp);
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", (int)temp.x, (int)temp.y, (int)temp.z]];
}

- (void)rotate90CW:(EAxis)theAxis center:(const TVector3f *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;

    if (!valid)
        [self validate];
    
    TVector3f temp;
    subV3f([self origin], theCenter, &temp);
    rotate90CWV3f(&temp, theAxis, &temp);
    addV3f(&temp, theCenter, &temp);
    roundV3f(&temp, &temp);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", (int)temp.x, (int)temp.y, (int)temp.z]];

    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotate90CWV3f(&direction, theAxis, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }
        
        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }

}

- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3f *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;

    if (!valid)
        [self validate];
    
    TVector3f temp;
    subV3f([self origin], theCenter, &temp);
    rotate90CCWV3f(&temp, theAxis, &temp);
    addV3f(&temp, theCenter, &temp);
    roundV3f(&temp, &temp);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", (int)temp.x, (int)temp.y, (int)temp.z]];

    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotate90CCWV3f(&direction, theAxis, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }
        
        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }

}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theRotationCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];

    TVector3f temp, offset;
    subV3f(&center, [self origin], &offset);
    subV3f(&center, theRotationCenter, &center);
    rotateQ(theRotation, &center, &center);
    addV3f(&center, theRotationCenter, &center);
    subV3f(&center, &offset, &temp);
    roundV3f(&temp, &temp);

    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", (int)temp.x, (int)temp.y, (int)temp.z]];
    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];

    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotateQ(theRotation, &direction, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }

        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }
}

- (void)flipAxis:(EAxis)theAxis center:(const TVector3f *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];
    
    TVector3f temp, offset;
    subV3f(&center, [self origin], &offset);
    subV3f(&center, theCenter, &center);
    
    switch (theAxis) {
        case A_X:
            center.x *= -1;
            break;
        case A_Y:
            center.y *= -1;
            break;
        default:
            center.z *= -1;
            break;
    }
    
    addV3f(&center, theCenter, &center);
    addV3f(&center, &offset, &temp);
    roundV3f(&temp, &temp);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", (int)temp.x, (int)temp.y, (int)temp.z]];
    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    if (a >= 0)
        a = (a + 180) % 360;
    else if (a == -1)
        a = -2;
    else if (a == -2)
        a = -1;
    [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
}

- (void)replaceProperties:(NSDictionary *)theProperties {
    NSAssert(theProperties != nil, @"properties must not be nil");
    
    NSString* newClassname = [theProperties objectForKey:ClassnameKey];
    if (![[self classname] isEqualToString:newClassname])
        [NSException raise:NSInvalidArgumentException format:@"new property set contains no or different classname"];
    
    [properties removeAllObjects];
    valid = NO;
    [angle release];
    angle = nil;
    
    for (NSString* key in theProperties) {
        NSString* value = [theProperties objectForKey:key];
        [self setProperty:key value:value];
    }
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey] && [self classname] != nil) {
        NSLog(@"Cannot overwrite classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        if (!parseV3f(value, NSMakeRange(0, [value length]), &origin)) {
            NSLog(@"Invalid origin value: '%@'", value);
            return;
        }
        roundV3f(&origin, &origin);
        valid = NO;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        if (value != nil)
            angle = [[NSNumber alloc] initWithInt:[value intValue]];
        else
            angle = nil;
    }
    
    NSString* oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];
    valid = NO;
}

- (void)removeProperty:(NSString *)key {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey]) {
        NSLog(@"Cannot delete classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        NSLog(@"Cannot delete origin property");
        return;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        angle = nil;
    }

    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
    valid = NO;
}

- (void)setEntityDefinition:(EntityDefinition *)theEntityDefinition {
    NSAssert(theEntityDefinition != nil, @"entity definition must not be nil");
    NSAssert(entityDefinition == nil, @"can't change entity definition");
    
    entityDefinition = [theEntityDefinition retain];
    [entityDefinition incUsageCount];
}

- (void)setMap:(id <Map>)theMap {
    map = theMap;
}

- (void)setSelected:(BOOL)isSelected {
    selected = isSelected;
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
}

- (NSString *)description {
    return [properties description];
}

#pragma mark -
#pragma mark @implementation Entity

- (NSNumber *)entityId {
    return entityId;
}

- (id <Map>)map {
    return map;
}

- (NSArray *)brushes {
    return brushes;
}

- (NSString *)propertyForKey:(NSString *)key {
    return (NSString *)[properties objectForKey:key];
}

- (NSDictionary *)properties {
    return properties;
}

- (BOOL)isPropertyDeletable:(NSString *)theKey {
    return ![ClassnameKey isEqualToString:theKey] && ![OriginKey isEqualToString:theKey] && ![SpawnFlagsKey isEqualToString:theKey];
}

- (BOOL)isPropertyWritable:(NSString *)theKey {
    return ![ClassnameKey isEqualToString:theKey];
}

- (NSString *)spawnFlagsString {
    if (entityDefinition == nil)
        return @"<missing entity definition>";
    
    NSString* raw = [properties objectForKey:SpawnFlagsKey];
    if (raw == nil || [raw intValue] == 0)
        return @"<none>";
    
    int mask = [raw intValue];
    NSArray* spawnFlags = [entityDefinition flagsForMask:mask];
    if ([spawnFlags count] == 0)
        return @"<none>";
    
    return [spawnFlags componentsJoinedByString:@" "];
}

- (NSString *)classname {
    return [self propertyForKey:ClassnameKey];
}

- (EntityDefinition *)entityDefinition {
    return entityDefinition;
}

- (BOOL)isWorldspawn {
    return [[self classname] isEqualToString:WorldspawnClassname];
}

- (const TBoundingBox *)maxBounds {
    if (!valid)
        [self validate];
    
    return &maxBounds;
}

- (const TBoundingBox *)bounds {
    if (!valid)
        [self validate];
    
    return &bounds;
}

- (const TVector3f *)center {
    if (!valid)
        [self validate];

    return &center;
}

- (const TVector3f *)origin {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        [NSException raise:NSInternalInconsistencyException format:@"Entity is not a point entity (ID %@)", entityId];
    
    return &origin;
}

- (NSNumber *)angle {
    return angle;
}

- (BOOL)selected {
    return selected;
}

- (void)pick:(const TRay *)theRay hitList:(PickingHitList *)theHitList {
    if ([self isWorldspawn])
        return;
    
    float dist = intersectBoundsWithRay([self bounds], theRay, NULL);
    if (isnan(dist))
        return;
    
    TVector3f hitPoint;
    rayPointAtDistance(theRay, dist, &hitPoint);

    PickingHit* pickingHit = [[PickingHit alloc] initWithObject:self type:HT_ENTITY hitPoint:&hitPoint distance:dist];
    [theHitList addHit:pickingHit];
    [pickingHit release];
}


- (VboBlock *)boundsVboBlock {
    return boundsVboBlock;
}

- (void)setBoundsVboBlock:(VboBlock *)theBoundsVboBlock {
    if (boundsVboBlock != NULL)
        freeVboBlock(boundsVboBlock);
    boundsVboBlock = theBoundsVboBlock;
}
@end
